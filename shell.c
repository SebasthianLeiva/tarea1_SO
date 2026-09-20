
#define _CRT_SECURE_NO_WARNINGS

#include "stdbool.h"
#include "stdio.h"
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <string.h>
#include <fcntl.h>

#include "redireccion/redireccion.h"
#include "parsing/parser.h"
#include "background/background.h"
#include "signals/signals.h"
#include "pipes/pipes.h"

#include "shell.h"
#include "pmon/pmon.h"



void shell()
{

    int cant_argumentos = 100;

    // lista de procesos background (para el built_in "jobs")

    struct Proceso_background lista_pros_background[100];

    // guarda la posicion en la que insertar el siguiente proceso background

    int cant_pros_background = 0;
    inicializar_background(lista_pros_background, 100);

    // La shell principal ignora Ctrl+C y Ctrl+\.
    configurar_senales_shell();



    while (true)
    {
        // notifica si algun proceso background termino desde la ultima llamada a jobs o al inicio de la shell
        notificar_background(lista_pros_background, cant_pros_background);

        // imprime el directorio en el prompt
        char directorio[1024];

        getcwd(directorio, sizeof(directorio));

        printf("%s ", directorio);

        char entrada_usuario[1024];

        // obtiene la entrada y la guarda en "entrada_usuario"

        // si se oprime ctrl + D el programa termina

        if (fgets(entrada_usuario, sizeof(entrada_usuario), stdin) == NULL)
        {

            return;
        }

        // array para el programa y argumentos

        char *args[cant_argumentos];


        //se obtienen el proceso y sus argumentos de la entrada del usuario

        parsear_entrada(entrada_usuario,args,cant_argumentos);



        // VALIDACION DE ENTRADA

        // si es que el usuario oprime "espacio" el while pasa a la siguiente iteracion
        // y la shell vuelve a pedir entrada

        if (args[0] == NULL)
        {

            continue;
        }

        // si el usuario escribe exit termina el programa

        else if (strcmp(args[0], "exit") == 0)
        {

            exit(0);
        }

        //Built-in "cd"

        else if (strcmp(args[0], "cd") == 0)
        {

            if (args[1] == NULL)
            {

                // si no se da directorio destino este se toma como Home

                chdir(getenv("HOME")); // getenv obtiene el directorio home del usuario al
                                       // darle "HOME"
            }

            else{   

                //cambia el directorio actual a la ruta dada por el usuario

                chdir(args[1]);

            }

        }

        
        //Built-in "jobs"

        else if (strcmp(args[0], "jobs") == 0)
        {

            jobs(cant_pros_background,lista_pros_background);

        }

        //Built-in "pmon"

        else if (strcmp(args[0], "pmon") == 0)
        {

            //se define cada cuantos segundos pmon se debe ejecutar

            int segundos = 2; 

            if(args[1]!=NULL){ 

                segundos = atoi(args[1]);
            }

            //TODO: implementar limpieza de la impresion, para que no se imprima una abajo de otra

            //TODO: refrescar periodicamente usando alarm() y SIGALARM

            pmon(lista_pros_background,cant_pros_background,segundos);

        }


        else{ // si el comando es diferente a los anteriores se ejecuta fork + execvp

    // Pasa a true si el comando debe ejecutarse en background.
    bool background = verificar_background(args);

    // Verifica si la entrada contiene uno o más pipes.
    bool tiene_pipe = verificar_pipe(args);

    /*
     * Los comandos con pipe necesitan varios procesos hijos,
     * por lo que se ejecutan mediante el módulo pipes en lugar
     * del fork + execvp normal de la shell.
     */
     
        if (tiene_pipe)
        {
            if (background)
            {
                limpiar_background(args);
            }

            /*
            * Guardamos el comando completo antes de separar la tubería,
            * para poder registrarlo posteriormente en jobs.
            */
            char comando[1024];
            reconstruir_comando(args, comando);

            char **comandos[100];
            pid_t pids_pipeline[100];

            int cantidad_comandos =
                separar_comandos_pipe(args, comandos, 100);

            /*
            * SIGCHLD permanece bloqueado mientras se crean y registran
            * los procesos para evitar que terminen antes del registro.
            */
            bloquear_sigchld();

            int cantidad_pids =
                ejecutar_pipeline(
                    comandos,
                    cantidad_comandos,
                    background,
                    pids_pipeline
                );

            if (cantidad_pids == -1)
            {
                desbloquear_sigchld();
                continue;
            }

            if (background)
            {
                int numero_job =
                    registrar_pipeline_background(
                        lista_pros_background,
                        &cant_pros_background,
                        100,
                        pids_pipeline,
                        cantidad_pids,
                        comando
                    );

                if (numero_job >= 0)
                {
                    printf(
                        "[%d] %d\n",
                        numero_job,
                        pids_pipeline[0]
                    );
                }
            }

            desbloquear_sigchld();

            continue;
        }


    /*
     * Desde aquí sigue el funcionamiento normal para comandos
     * que NO contienen pipes.
     */

    char *archivo_entrada = NULL;
    char *archivo_salida = NULL;

    int append = 0;

    verificar_operadores_redir(
        &append,
        args,
        &archivo_salida,
        &archivo_entrada
    );

            bloquear_sigchld();
            
            __pid_t pid = fork(); // se crea el proceso hijo

            if (pid == 0)
            {
                // El hijo ya no necesita mantener SIGCHLD bloqueado.
                desbloquear_sigchld();

                /*
                * Los procesos background se colocan en un grupo de procesos propio,
                * evitando que reciban Ctrl+C dirigido al grupo foreground de la shell.
                */
                if (background)
                {
                    if (setpgid(0, 0) == -1)
                    {
                        perror("setpgid");
                        _exit(1);
                    }
                }

                /*
                * fork() hereda la configuración de señales de la shell.
                * Restauramos SIGINT y SIGQUIT para que el programa ejecutado
                * tenga el comportamiento normal de un proceso Linux.
                */
                restaurar_senales_hijo();

                // Se redirecciona la entrada y/o salida si corresponde.
                redireccionar_entrada_salida(
                    archivo_salida,
                    archivo_entrada,
                    append
                );

                if (archivo_salida != NULL || archivo_entrada != NULL)
                {
                    limpiar_redireccion_args(args);
                }

                limpiar_background(args);

                execvp(args[0], args);

                perror("execvp");
                _exit(127);
            }

            // proceso background

            else if ((pid > 0) && background == true){

                /*
                 * El padre también intenta colocar al hijo background en su propio
                 * grupo para evitar condiciones de carrera con el hijo.
                 */
                if (setpgid(pid, pid) == -1)
                    {perror("setpgid");}

                char comando[1024]; // string vacio

                // se quita <,>,>> y & de args

                limpiar_background(args);
                limpiar_redireccion_args(args);

                reconstruir_comando(args, comando); // reconstruye cadena de comandos

                int numero_job = registrar_background(lista_pros_background,&cant_pros_background, 100, pid, comando);

                if (numero_job >= 0)
                    printf("[%d] %d\n", numero_job, pid);
                desbloquear_sigchld();
            }


            // proceso del padre (shell) 

            else if (pid > 0 && background == false){ 

                int status;

                desbloquear_sigchld();
                waitpid(pid, &status, 0); // espera al hijo
            }

            // el programa termina ante error fatal de fork 

            else{ 

                desbloquear_sigchld();
                perror("fork");

                return;
            }
        }
    }
}


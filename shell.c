
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

#include "shell.h"


void shell()
{

    int cant_argumentos = 100;

    // lista de procesos background (para el built_in "jobs")

    struct Proceso_background lista_pros_background[100];

    // guarda la posicion en la que insertar el siguiente proceso background

    int cant_pros_background = 0;

    while (true)
    {

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

        else if (strcmp(args[0], "jobs") == 0)
        {

            jobs(cant_pros_background,lista_pros_background);

        }

        else if (strcmp(args[0], "pmon") == 0)
        {

            // ejecutar pmon directamente
        }

        // si el comando es diferecte a los anteriores se ejecuta fork + execvp

        else
        {

            // pasa a true en caso de encontrarse & (programa a ejecutar en background)

            bool background = false;

            background = verificar_background(args);

            // guardan el archivo de entrada y salida para poder abrirlos con open()

            char *archivo_entrada = NULL;
            char *archivo_salida = NULL;

            int append = 0; // cambia a 1 si se encuentra >> (escritura con append)

            // se verifica si la entrada solicita redireccion con: < , > o >>

            verificar_operadores_redir(&append, args, &archivo_salida, &archivo_entrada);

            __pid_t pid = fork(); // se crea el proceso hijo

            if (pid == 0)
            {

                // //se redirecciona la entrada y/o salida en caso de ser necesario

                redireccionar_entrada_salida(archivo_salida, archivo_entrada, append);

                if (archivo_salida != NULL || archivo_entrada != NULL)
                {

                    limpiar_redireccion_args(args);
                }

                limpiar_background(args);

                // el hijo pasa a ejecutar el proceso del primer argumento de args
                // usando como argumentos los posteriores a el

                execvp(args[0], args);

                perror("execvp"); /* solo si exec falla */

                _exit(127);
            }

            // proceso background

            else if ((pid > 0) && background == true)
            {

                // se agrega el proceso background a la lista con estado inicial "Ejecutando"

                char comando[1024]; // string vacio

                // se quita <,>,>> y & de args

                limpiar_background(args);
                limpiar_redireccion_args(args);

                reconstruir_comando(args, comando); // reconstruye cadena de comandos

                lista_pros_background[cant_pros_background].pid = pid;

                strcpy(lista_pros_background[cant_pros_background].comando, comando);

                strcpy(lista_pros_background[cant_pros_background].estado, "Ejecutando");

                // actualiza la posicion actual del primer elemento vacio de la lista de
                // procesos background

                cant_pros_background++;
            }

            else if (pid > 0 && background == false)
            { // proceso del padre (shell)

                int status;

                waitpid(pid, &status, 0); // espera al hijo
            }

            else
            { // el programa termina ante error fatal de fork

                perror("fork");

                return;
            }
        }
    }
}



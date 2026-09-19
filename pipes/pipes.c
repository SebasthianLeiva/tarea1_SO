#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "pipes.h"
#include "../signals/signals.h"


int ejecutar_pipeline(char **comandos[], int cantidad_comandos, bool background)
{

//Una tubería de N comandos necesita N-1 pipes

    int cantidad_pipes = cantidad_comandos - 1;

    /*
     * Cada pipe posee dos descriptores:
     * [0] lectura
     * [1] escritura
     */
    int pipes[cantidad_pipes][2];

    pid_t pids[cantidad_comandos];

    // Se crean todos los pipes antes de crear los procesos hijos

    for (int i = 0; i < cantidad_pipes; i++)
    {
        if (pipe(pipes[i]) == -1)
        {
            perror("pipe");
            return -1;
        }
    }

    // Cada comando de la tubería se ejecuta en un proceso distinto
    for (int i = 0; i < cantidad_comandos; i++)
    {
        pid_t pid = fork();

        if (pid == -1)
        {
            perror("fork");

            // Cerramos todos los pipes creados antes de retornar
            for (int j = 0; j < cantidad_pipes; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            return -1;
        }

        if (pid == 0)
        {
            /*
             * Los hijos recuperan el comportamiento normal de
             * SIGINT y SIGQUIT antes de ejecutar el comando
             */
            restaurar_senales_hijo();

            /*
             * Si no es el primer comando, su entrada estándar
             * debe venir del extremo de lectura del pipe anterior
             */
            if (i > 0)
            {
                if (dup2(pipes[i - 1][0], STDIN_FILENO) == -1)
                {
                    perror("dup2 stdin");
                    _exit(1);
                }
            }

            /*
             * Si no es el último comando, su salida estándar
             * debe dirigirse al extremo de escritura del pipe actual
             */
            if (i < cantidad_comandos - 1)
            {
                if (dup2(pipes[i][1], STDOUT_FILENO) == -1)
                {
                    perror("dup2 stdout");
                    _exit(1);
                }
            }

            /*
             * Después de dup2(), los descriptores originales ya no
             * son necesarios. Todos los hijos deben cerrarlos
             */
            for (int j = 0; j < cantidad_pipes; j++)
            {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execvp(comandos[i][0], comandos[i]);

            // execvp() solo retorna si ocurre un error
            perror("execvp");
            _exit(127);
        }

        // El padre guarda cada PID para poder esperarlos después
        pids[i] = pid;
    }

    /*
     * El padre tampoco debe mantener abiertos los pipes
     * De lo contrario un lector podría no recibir EOF
     */
    for (int i = 0; i < cantidad_pipes; i++)
    {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    /*
     * En foreground la shell espera que termine toda la tubería
     * antes de volver a mostrar el prompt
     */
    if (!background)
    {
        for (int i = 0; i < cantidad_comandos; i++)
        {
            waitpid(pids[i], NULL, 0);
        }
    }

    return 0;
}
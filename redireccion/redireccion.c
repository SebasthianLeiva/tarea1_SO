
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "redireccion.h"

void redireccionar_entrada_salida(char *archivo_salida, char *archivo_entrada, int append){

    // redireccion de salida

    if (archivo_salida != NULL)
    { // redireccionamos la salida a archivo

        int fd; // descriptor del archivo que se abrira

        if (append == 0)
        {

            // se abre el archivo con permisos de escritura, creacion y truncamiento

            fd = open(archivo_salida, O_WRONLY | O_CREAT | O_TRUNC, 0644);

            if (fd == -1)
            {
                perror("open");
                _exit(1);
            }
        }

        else
        {

            // se abre el archivo en modo append

            fd = open(archivo_salida, O_WRONLY | O_CREAT | O_APPEND, 0644);

            if (fd == -1)
            {
                perror("open");
                _exit(1);
            }
        }

        dup2(fd, STDOUT_FILENO); // el descriptor stdout pasa a apuntar al del archivo que se
                                 //  abrio lo que permite escribir la salida ahi

        close(fd);
    }

    // redireccion de entrada

    if (archivo_entrada != NULL)
    {

        int fd = open(archivo_entrada, O_RDONLY); // se abre el archivo de entrada para lectura

        dup2(fd, STDIN_FILENO); // el descriptor stdin apunta al archivo

        close(fd);
    }

}



void limpiar_redireccion_args(char *args[])
{

    int j = 0;

    for (int i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], ">") == 0 || strcmp(args[i], ">>") == 0 ||
            strcmp(args[i], "<") == 0)
        {

            i++; // saltar tambien el nombre del archivo
        }

        else
        {

            args[j] = args[i];
            j++;
        }
    }

    args[j] = NULL;
}



// obtiene el archivo entrada y/o salida en caso de redireccion

void verificar_operadores_redir(
    int *append,
    char *args[],
    char **archivo_salida,
    char **archivo_entrada)
{
    for (int i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], ">") == 0)
        {
            *archivo_salida = args[i + 1];
        }

        else if (strcmp(args[i], ">>") == 0)
        {
            *archivo_salida = args[i + 1];
            *append = 1;
        }

        else if (strcmp(args[i], "<") == 0)
        {
            *archivo_entrada = args[i + 1];
        }
    }
}




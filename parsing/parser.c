
#include <stdbool.h>
#include <string.h>

#include "parser.h"



void parsear_entrada(char *entrada_usuario, char *args[], int max_argumentos)
{
    char *token = strtok(entrada_usuario, " \n");

    int i = 0;

    while (token != NULL && i < max_argumentos - 1)
    {
        args[i] = token;

        i++;

        token = strtok(NULL, " \n");
    }

    args[i] = NULL;
}



// quita el '&' del arreglo de argumentos

void limpiar_background(char *args[])
{
    int j = 0;

    for (int i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], "&") == 0)
        {
            continue; // no copiar &
        }

        args[j] = args[i];
        j++;
    }

    args[j] = NULL;
}





// devuelve true si se solicita ejecutar el programa como background (al encotnrar &)

bool verificar_background(char *args[])
{

    for (int i = 0; args[i] != NULL; i++)
    {

        if (strcmp(args[i], "&") == 0)
        {

            return true;
        }
    }

    return false;
}

// Devuelve true si existe al menos un operador pipe en los argumentos.
bool verificar_pipe(char *args[])
{
    for (int i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], "|") == 0)
        {
            return true;
        }
    }

    return false;
}


/*
 * Separa una secuencia de argumentos en comandos independientes.
 *
 * Los "|" se reemplazan por NULL, lo que permite usar cada segmento
 * directamente como arreglo de argumentos para execvp().
 */
int separar_comandos_pipe(char *args[], char **comandos[], int max_comandos)
{
    int cantidad = 0;

    if (args[0] == NULL)
    {
        return 0;
    }

    // El primer comando siempre comienza en args[0].
    comandos[cantidad++] = &args[0];

    for (int i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], "|") == 0)
        {
            // Finaliza el argv del comando anterior.
            args[i] = NULL;

            // El siguiente argumento pasa a ser el inicio del próximo comando.
            if (args[i + 1] != NULL && cantidad < max_comandos)
            {
                comandos[cantidad++] = &args[i + 1];
            }
        }
    }

    return cantidad;
}


// toma la lista de argumentos (args) y reconstruye la cadena sin los operadores (&,<,>,>>)

void reconstruir_comando(char *args[], char *comando)
{

    comando[0] = '\0';

    for (int i = 0; args[i] != NULL; i++)
    {

        strcat(comando, args[i]);

        if (args[i + 1] != NULL)
        {

            strcat(comando, " ");
        }
    }
}



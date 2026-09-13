
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



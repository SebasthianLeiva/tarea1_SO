

#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>


// se parsea la entrada y se insertan los tokens en el array

void parsear_entrada(char *entrada_usuario, char *args[], int max_argumentos);


// quita el '&' del arreglo de argumentos

void limpiar_background(char *args[]);



// devuelve true si se solicita ejecutar el programa como background (al encotnrar &)

bool verificar_background(char *args[]);



// toma la lista de argumentos (args) y reconstruye la cadena sin los operadores (&,<,>,>>)

void reconstruir_comando(char *args[], char *comando);

// Devuelve true si la entrada contiene al menos un operador pipe '|'.
bool verificar_pipe(char *args[]);

/*
 * Separa los argumentos en comandos individuales usando "|" como delimitador.
 * Cada "|" se reemplaza por NULL para que cada comando pueda utilizarse
 * directamente como argv en execvp().
 *
 * Retorna la cantidad de comandos encontrados.
 */
int separar_comandos_pipe(char *args[], char **comandos[], int max_comandos);

#endif
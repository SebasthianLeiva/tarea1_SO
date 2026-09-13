

#ifndef PARSER_H
#define PARSER_H


// se parsea la entrada y se insertan los tokens en el array

void parsear_entrada(char *entrada_usuario, char *args[], int max_argumentos);


// quita el '&' del arreglo de argumentos

void limpiar_background(char *args[]);



// devuelve true si se solicita ejecutar el programa como background (al encotnrar &)

bool verificar_background(char *args[]);



// toma la lista de argumentos (args) y reconstruye la cadena sin los operadores (&,<,>,>>)

void reconstruir_comando(char *args[], char *comando);



#endif
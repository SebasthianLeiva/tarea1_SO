#ifndef PIPES_H
#define PIPES_H

#include <stdbool.h>

/*
 * Ejecuta una tubería compuesta por uno o más comandos.
 *
 * Crea N-1 pipes para N comandos y ejecuta cada comando
 * en un proceso hijo independiente.
 */
int ejecutar_pipeline(char **comandos[], int cantidad_comandos, bool background);

#endif
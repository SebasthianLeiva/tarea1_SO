#ifndef PIPES_H
#define PIPES_H

#include <stdbool.h>
#include <sys/types.h>

/*
 * Ejecuta una tubería compuesta por uno o más comandos.
 *
 * Crea N-1 pipes para N comandos y ejecuta cada comando
 * en un proceso hijo independiente.
 * pids_salida recibe los PID de cada proceso creado.
 * Retorna la cantidad de procesos creados, o -1 ante error.
 */

int ejecutar_pipeline(char **comandos[], int cantidad_comandos, bool background, pid_t pids_salida[]);

#endif
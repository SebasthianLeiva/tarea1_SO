

#ifndef BACKGROUND_H
#define BACKGROUND_H


//representa un proceso background

struct Proceso_background
{

    pid_t pid;
    char comando[1024];
    char estado[20];
};


//se ejecuta con el comando "jobs" , muestra los procesos background almacenados

void jobs(int cant_pros_background, struct Proceso_background lista_pros_background[]);

#endif


#ifndef BACKGROUND_H
#define BACKGROUND_H


// permite almacenar la informacion de procesos background para luego listarlos con job

struct Proceso_background
{

    pid_t pid;
    char comando[1024];
    char estado[20];
};


//a ejecutar con el comando "jobs" , muestra los procesos background

void jobs(int cant_pros_background, struct Proceso_background lista_pros_background[]);

#endif
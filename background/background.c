


#include <stdio.h>     
#include <string.h>    
#include <unistd.h>   
#include <sys/types.h> 
#include <sys/wait.h> 

#include "background.h"


void jobs(int cant_pros_background, struct Proceso_background lista_pros_background[]){

    int status; // waitpid() le asigna informacion del proceso 

    //recorre todos los procesos background de la lista, se actualiza su estado y se imprime su PID, comandos y estado

    for (int i = 0; i < cant_pros_background; i++){


        pid_t resultado = waitpid(lista_pros_background[i].pid, &status, WNOHANG);

        if (resultado == 0)
        {

            strcpy(lista_pros_background[i].estado, "Ejecutando");
        }

        else if (resultado == lista_pros_background[i].pid)
        {

            strcpy(lista_pros_background[i].estado, "Terminado");
        }

        else
        {

            strcpy(lista_pros_background[i].estado, "Error");
        }

        printf("PID: %d\n", lista_pros_background[i].pid);
        printf("Comando: %s\n", lista_pros_background[i].comando);
        printf("Estado: %s\n", lista_pros_background[i].estado);

    }


}


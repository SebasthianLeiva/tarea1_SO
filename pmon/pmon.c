#include "pmon.h"

#include <unistd.h>
#include <string.h>
#include <stdlib.h>



int obtener_datos_stat(pid_t pid, char *estado,unsigned long *utime,unsigned long *stime);


void pmon(struct Proceso_background lista_pros_background[],int cant_procesos_background, int segundos){

    printf("%-8s %-25s %-12s %-12s %-10s\n", "PID", "COMANDO", "ESTADO", "CPU(%)", "RSS(KB)");

    long clk_tck = sysconf(_SC_CLK_TCK); //necesario para obtener CPU%

    for(int i=0;i<cant_procesos_background;i++){

        char estado;
        unsigned long utime;
        unsigned long stime;    

        //si la lectura de /stat falla se va al siguiente

        if(obtener_datos_stat(lista_pros_background[i].pid,&estado,&utime,&stime)==-1){ 

            continue;

        };


        char estado_texto[50];

        switch (estado) {
        
        case 'R':
        strcpy(estado_texto,"ejecutando");
        break;

        case 'S':
        strcpy(estado_texto,"durmiendo");
        break;

        case 'Z':
        strcpy(estado_texto,"zombie");
        break;

        case 'T':
        strcpy(estado_texto,"detenido");
        break;

        //falta caso default

        default:

        strcpy(estado_texto,"-");

        }


        char porcentajeCPU_texto[50];


        unsigned long ticks_cpu_anterior = lista_pros_background[i].ticks_cpu; //los ticks cpu que hasta ahora tenia el proceso

        unsigned long ticks_cpu_actual = utime + stime; //ticks cpu calculados en la iteracion actual


        if (lista_pros_background[i].tiene_medicion_cpu== true){ //si es que no se esta en la primera medicion se usan los ticks_cpu guardados 
                                    //de la iteracion anterior para restarlos a los de la actual y obtener delta_cpu

        unsigned long delta_cpu = ticks_cpu_actual - ticks_cpu_anterior;

        //se obtiene %CPU

        double porcentajeCPU = ((double)delta_cpu / clk_tck)/(segundos)* 100.0; 

        snprintf(porcentajeCPU_texto,sizeof(porcentajeCPU_texto),"%.2f",porcentajeCPU);

        }

        else {
        strcpy(porcentajeCPU_texto,"-"); //si se esta en la primera medicion no se puede calular %CPU y se imprime '-'
        }


        //TODO: llamar a "obtener_datos_status" para obtener rss 

        
        printf("%-8d %-25s %-12s %-12s \n",
        lista_pros_background[i].pid,
        lista_pros_background[i].comando,
        estado_texto,
        porcentajeCPU_texto);
        //imprimir rss


        lista_pros_background[i].ticks_cpu = ticks_cpu_actual; //se actualizan los ticks cpu del proceso

        lista_pros_background[i].tiene_medicion_cpu = true;





    }
    

}




int obtener_datos_stat(pid_t pid, char *estado,
                       unsigned long *utime,
                       unsigned long *stime)
{
    char ruta[60]; //array de char contenedor de la ruta del archivo con el pid concatenado
    char linea[1000]; //array de char contenedor de la linea

    snprintf(ruta, sizeof(ruta), "/proc/%d/stat", pid);

    FILE *archivo = fopen(ruta, "r"); //si no se pudo abrir el archivo
    if (archivo == NULL)
    {
        return -1;
    }

    //se lee solo la primera linea de /proc/pid/stat (ya que es en la cual se encuentra state y la informacion para %CPU)
    //y se guarda en el array de char "linea"

    if (fgets(linea, sizeof(linea), archivo) == NULL){  //si no se pudo leer se cierra el archivo y el metodo etorna 

        fclose(archivo);
        return -1;
    }

    fclose(archivo);

    //se usa strrchr() para buscar el primer caracter ')' partiendo desde el final de la linea, dado que el campo 
    // "state" es el 3er campo, el 2do es el nombre del proceso y puede contener ( ) , por lo que leer desde el inicio
    //podria llevar a errores. Mientras que tras el 3er campo todos los campos son numericos por lo cual no existiran 
    //errores de lectura al empezar desde atras
  
    char *caracter_cierre = strrchr(linea, ')'); 

    if (caracter_cierre == NULL){ //si por alguna razon no se encuentra ')'
    return -1;
    }

    //puntero al caracter donde empieza la parte de la linea que nos interesa

    char *resto = caracter_cierre + 2;

    int campo = 3; 

    char *token = strtok(resto, " "); //comienza a separar "resto" y obtiene el primer token

    while (token != NULL){

        if (campo == 3){ 

            *estado = token[0]; //se obtiene state
        }

        else if (campo == 14)
        {
            *utime = strtoul(token, NULL, 10); //se obtiene utime como unsigned long y se guarda como valor del puntero utime
        }
        else if (campo == 15)
        {
            *stime = strtoul(token, NULL, 10); //se obtiene stime unsigned long y se guarda como valor del puntero stime
        }

        if (campo == 15)
        {
            break;
        }

        campo++;
        token = strtok(NULL, " "); //continua con el resto 

    }


    return 0;
}


int obtener_datos_status(){

    //TODO: obrir el archivo /proc/pid/status y obtener rss

    return 0;

}
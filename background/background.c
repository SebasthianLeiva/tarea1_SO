#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "background.h"
#include <stdbool.h>

static struct Proceso_background *lista_global; // puntero a la lista de procesos background
static volatile sig_atomic_t terminados[100]; // array que indica si un proceso background ha terminado (1) o no (0)
static int capacidad_global;

static void manejar_sigchld(int senal)
{
    int estado;
    pid_t pid;

    (void)senal; // evita advertencias de compilacion por variable no utilizada

    // el bucle maneja todos los procesos hijos que hayan terminado
    while ((pid = waitpid(-1, &estado, WNOHANG)) > 0){
        for (int i = 0; i < capacidad_global; i++){
            for (int j = 0; j < lista_global[i].cantidad_pids; j++)
            {
                if (lista_global[i].pids[j] == pid)
                {
                
                    // Uno de los procesos pertenecientes al job terminó.
    
                    lista_global[i].procesos_terminados++;

                    /*
                    * El job completo se considera terminado solamente
                    * cuando finalizaron todos sus procesos.
                    */
                    if (lista_global[i].procesos_terminados >= lista_global[i].cantidad_pids){
                        terminados[i] = 1;
                    }

                    break;
                }
            }
        }
    }
}

void inicializar_background(struct Proceso_background lista[], int capacidad)
{
    struct sigaction accion = {0};

    lista_global = lista;
    capacidad_global = capacidad < 100 ? capacidad : 100;

    // inicializa la lista de procesos background y el array de terminados
    memset(lista_global, 0, (size_t)capacidad_global * sizeof(lista_global[0]));
    memset((void *)terminados, 0, sizeof(terminados));

    // configura el manejador de la señal SIGCHLD
    accion.sa_handler = manejar_sigchld;
    sigemptyset(&accion.sa_mask);
    accion.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &accion, NULL) == -1) // maneja el SIGCHLD para saber cuando un proceso hijo termina
    {
        perror("sigaction");
    }
}

void bloquear_sigchld(void)
{
    sigset_t conjunto; // señales a bloquear
    sigemptyset(&conjunto);
    sigaddset(&conjunto, SIGCHLD);

    if (sigprocmask(SIG_BLOCK, &conjunto, NULL) == -1) // bloquea cuando se registra el proceso background
    {
        perror("sigprocmask");
    }
}

void desbloquear_sigchld(void)
{
    sigset_t conjunto;
    sigemptyset(&conjunto);
    sigaddset(&conjunto, SIGCHLD);

    if (sigprocmask(SIG_UNBLOCK, &conjunto, NULL) == -1) // analogo al anterior
    {
        perror("sigprocmask");
    }
}

int registrar_background(struct Proceso_background lista[], int *cantidad, int capacidad, pid_t pid, const char *comando)
{
    int indice;

    if (*cantidad >= capacidad)
    {
        fprintf(stderr, "Se alcanzo el limite de jobs en background\n");
        return -1;
    }

    indice = *cantidad;
    lista[indice].pid = pid;
    lista[indice].pids[0] = pid;
    lista[indice].cantidad_pids = 1;
    lista[indice].procesos_terminados = 0;
    lista[indice].numero = indice + 1;
    lista[indice].ticks_cpu= 0;
    lista[indice].tiene_medicion_cpu = false;

    // copia el comando y el estado a la estructura correspondiente
    snprintf(lista[indice].comando, sizeof(lista[indice].comando), "%s", comando);
    snprintf(lista[indice].estado, sizeof(lista[indice].estado), "Ejecutando");
    terminados[indice] = 0;
    (*cantidad)++;

    return lista[indice].numero; // devuelve el numero de job asignado
}

int registrar_pipeline_background(
    struct Proceso_background lista[],
    int *cantidad,
    int capacidad,
    pid_t pids[],
    int cantidad_pids,
    const char *comando)
{
    if (*cantidad >= capacidad)
    {
        fprintf(stderr, "Se alcanzo el limite de jobs en background\n");
        return -1;
    }

    if (cantidad_pids <= 0 || cantidad_pids > MAX_PROCESOS_JOB)
    {
        fprintf(stderr, "Cantidad invalida de procesos en pipeline\n");
        return -1;
    }

    int indice = *cantidad;

    /*
     * El primer proceso se mantiene como PID representativo
     * del job y también actuará como líder del grupo.
     */
    lista[indice].pid = pids[0];

    // Guardamos todos los procesos que forman la tubería.
    for (int i = 0; i < cantidad_pids; i++)
    {
        lista[indice].pids[i] = pids[i];
    }

    lista[indice].cantidad_pids = cantidad_pids;
    lista[indice].procesos_terminados = 0;

    lista[indice].numero = indice + 1;
    lista[indice].ticks_cpu = 0;
    lista[indice].tiene_medicion_cpu = false;

    snprintf(
        lista[indice].comando,
        sizeof(lista[indice].comando),
        "%s",
        comando
    );

    snprintf(
        lista[indice].estado,
        sizeof(lista[indice].estado),
        "Ejecutando"
    );

    terminados[indice] = 0;

    (*cantidad)++;

    return lista[indice].numero;
}

void notificar_background(struct Proceso_background lista[], int cantidad)
{
    for (int i = 0; i < cantidad; i++) // recorre la lista de procesos en background y si alguno termino lo notifica
    {
        if (terminados[i])
        {
            terminados[i] = 0;
            snprintf(lista[i].estado, sizeof(lista[i].estado), "Terminado");
            printf("[%d]+ Done %s\n", lista[i].numero, lista[i].comando);
            fflush(stdout); // para imprimir inmediatamente
        }
    }
}

void jobs(int cant_pros_background, struct Proceso_background lista_pros_background[]){
    notificar_background(lista_pros_background, cant_pros_background); // notifica si terminaron procesos en background
    for (int i = 0; i < cant_pros_background; i++){ // recorre la lista de procesos en background para desplegar su informacion

        printf("%-8s %-25s %-12s\n", "PID", "COMANDO", "ESTADO");

        printf("%-8d %-25s %-12s  \n",
        lista_pros_background[i].pid,
        lista_pros_background[i].comando,
        lista_pros_background[i].estado);

    }
}

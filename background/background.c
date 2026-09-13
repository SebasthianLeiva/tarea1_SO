#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "background.h"

static struct Proceso_background *lista_global; // puntero a la lista de procesos background
static volatile sig_atomic_t terminados[100]; // array que indica si un proceso background ha terminado (1) o no (0)
static int capacidad_global;

static void manejar_sigchld(int senal)
{
    int estado;
    pid_t pid;

    (void)senal; // evita advertencias de compilacion por variable no utilizada

    // el bucle maneja todos los procesos hijos que hayan terminado
    while ((pid = waitpid(-1, &estado, WNOHANG)) > 0)
    {
        for (int i = 0; i < capacidad_global; i++)
        {
            if (lista_global[i].pid == pid)
            {
                terminados[i] = 1;
                break;
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
    lista[indice].numero = indice + 1;

    // copia el comando y el estado a la estructura correspondiente
    snprintf(lista[indice].comando, sizeof(lista[indice].comando), "%s", comando);
    snprintf(lista[indice].estado, sizeof(lista[indice].estado), "Ejecutando");
    terminados[indice] = 0;
    (*cantidad)++;

    return lista[indice].numero; // devuelve el numero de job asignado
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
        printf("PID: %d\n", lista_pros_background[i].pid);
        printf("Comando: %s\n", lista_pros_background[i].comando);
        printf("Estado: %s\n", lista_pros_background[i].estado);
    }
}

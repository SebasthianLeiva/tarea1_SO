#ifndef BACKGROUND_H
#define BACKGROUND_H
#include <signal.h>
#include <sys/types.h>
#include <stdbool.h>
#define MAX_PROCESOS_JOB 100

//representa un proceso background
struct Proceso_background{

    // PID representativo del job.
    pid_t pid;

    /*
     * Un job puede contener varios procesos cuando se ejecuta
     * una tubería en background.
     */
    pid_t pids[MAX_PROCESOS_JOB];
    int cantidad_pids;
    int procesos_terminados;

    int numero;
    char comando[1024];
    char estado[20];
    unsigned long ticks_cpu;
    bool tiene_medicion_cpu;
};

// Registra una tubería completa como un único job en background.

int registrar_pipeline_background(
    struct Proceso_background lista_pros_background[],
    int *cant_pros_background,
    int capacidad,
    pid_t pids[],
    int cantidad_pids,
    const char *comando
);

// inicializa el registro y el manejador asincrono de SIGCHLD
void inicializar_background(struct Proceso_background lista_pros_background[], int capacidad);

// evita que SIGCHLD se procese entre fork() y el registro del nuevo job
void bloquear_sigchld(void);
void desbloquear_sigchld(void);

// registra un proceso que fue lanzado en background.
int registrar_background(struct Proceso_background lista_pros_background[], int *cant_pros_background, int capacidad, pid_t pid, const char *comando);

//imprime una notificacion por cada job terminado desde la ultima llamada.
void notificar_background(struct Proceso_background lista_pros_background[], int cant_pros_background);

//se ejecuta con el comando "jobs" , muestra los procesos background almacenados
void jobs(int cant_pros_background, struct Proceso_background lista_pros_background[]);



#endif
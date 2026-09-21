#include "pmon.h"

#include "../background/background.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static volatile sig_atomic_t refrescar;
static volatile sig_atomic_t salir;

// el temporizador solicita una nueva lectura.
static void manejar_sigalrm(int senal)
{
    (void)senal;
    refrescar = 1;
}

// ctrl+c solo termina el modo de monitoreo.
static void manejar_sigint(int senal)
{
    (void)senal;
    salir = 1;
}

static int obtener_datos_stat(pid_t pid,
                              char *estado,
                              unsigned long *utime,
                              unsigned long *stime)
{
    char ruta[64];
    char linea[4096];
    char *cierre;
    char *token;
    int campo = 3;
    FILE *archivo;

    snprintf(ruta, sizeof(ruta), "/proc/%d/stat", pid);
    archivo = fopen(ruta, "r");
    if (archivo == NULL)
        return -1;

    if (fgets(linea, sizeof(linea), archivo) == NULL)
    {
        fclose(archivo);
        return -1;
    }
    fclose(archivo);

    // el nombre del proceso puede contener espacios y parentesis
    cierre = strrchr(linea, ')');
    if (cierre == NULL || cierre[1] != ' ')
        return -1;

    token = strtok(cierre + 2, " ");
    while (token != NULL && campo <= 15)
    {
        if (campo == 3)
            *estado = token[0];
        else if (campo == 14)
            *utime = strtoul(token, NULL, 10);
        else if (campo == 15)
            *stime = strtoul(token, NULL, 10);

        token = strtok(NULL, " ");
        campo++;
    }

    return campo > 15 ? 0 : -1;
}

static int obtener_rss(pid_t pid, unsigned long *rss_kb)
{
    char ruta[64];
    char linea[256];
    unsigned long rss;
    FILE *archivo;

    // indica la memoria en kilobytes
    snprintf(ruta, sizeof(ruta), "/proc/%d/status", pid);
    archivo = fopen(ruta, "r");
    if (archivo == NULL)
        return -1;

    while (fgets(linea, sizeof(linea), archivo) != NULL)
    {
        if (sscanf(linea, "VmRSS: %lu kB", &rss) == 1)
        {
            fclose(archivo);
            *rss_kb = rss;
            return 0;
        }
    }

    fclose(archivo);
    return -1;
}

static const char *texto_estado(char estado)
{
    switch (estado)
    {
    case 'R':
        return "ejecutando";
    case 'S':
        return "durmiendo";
    case 'Z':
        return "zombie";
    case 'T':
        return "detenido";
    default:
        return "desconocido";
    }
}

static double segundos_transcurridos(const struct timespec *inicio,
                                     const struct timespec *fin)
{
    return (double)(fin->tv_sec - inicio->tv_sec) +
           (double)(fin->tv_nsec - inicio->tv_nsec) / 1000000000.0;
}

static void dibujar(struct Proceso_background lista[],
                    int cantidad,
                    long ticks_por_segundo,
                    double intervalo)
{
    printf("\033[H\033[2J");
    printf("%-8s %-25s %-15s %-12s %-10s\n",
           "PID", "COMANDO", "ESTADO", "CPU(%)", "RSS(KB)");

    for (int i = 0; i < cantidad; i++)
    {
        char estado;
        unsigned long utime;
        unsigned long stime;
        unsigned long rss;
        unsigned long ticks_actual;
        char cpu[32];

        if (lista[i].estado[0] == '\0' ||
            obtener_datos_stat(lista[i].pid, &estado, &utime, &stime) == -1)
            continue;

        if (obtener_rss(lista[i].pid, &rss) == -1)
            rss = 0;

        ticks_actual = utime + stime;
        // la primera lectura solo sirve como referencia.
        if (lista[i].tiene_medicion_cpu && intervalo > 0.0)
        {
            double cpu_porcentaje =
                ((double)(ticks_actual - lista[i].ticks_cpu) /
                 (double)ticks_por_segundo) /
                intervalo * 100.0;
            snprintf(cpu, sizeof(cpu), "%.2f", cpu_porcentaje);
        }
        else
        {
            snprintf(cpu, sizeof(cpu), "-");
        }

        printf("%-8d %-25s %-15s %-12s %-10lu\n",
               lista[i].pid,
               lista[i].comando,
               texto_estado(estado),
               cpu,
               rss);

        lista[i].ticks_cpu = ticks_actual;
        lista[i].tiene_medicion_cpu = true;
    }
    fflush(stdout);
}

void pmon(struct Proceso_background lista[], int cantidad, int segundos)
{
    struct sigaction accion_alarm = {0};
    struct sigaction accion_int = {0};
    struct sigaction anterior_alarm;
    struct sigaction anterior_int;
    struct timespec anterior;
    // los tiempos de cpu de /proc se expresan en ticks
    long ticks_por_segundo = sysconf(_SC_CLK_TCK);

    if (segundos <= 0)
    {
        fprintf(stderr, "pmon: el intervalo debe ser mayor que cero\n");
        return;
    }
    if (ticks_por_segundo <= 0)
    {
        fprintf(stderr, "pmon: no se pudo obtener _SC_CLK_TCK\n");
        return;
    }

    refrescar = 1;
    salir = 0;
    accion_alarm.sa_handler = manejar_sigalrm;
    sigemptyset(&accion_alarm.sa_mask);
    accion_alarm.sa_flags = 0;

    accion_int.sa_handler = manejar_sigint;
    sigemptyset(&accion_int.sa_mask);
    accion_int.sa_flags = 0;

    if (sigaction(SIGALRM, &accion_alarm, &anterior_alarm) == -1 ||
        sigaction(SIGINT, &accion_int, &anterior_int) == -1)
    {
        perror("pmon: sigaction");
        return;
    }

    clock_gettime(CLOCK_MONOTONIC, &anterior);
    // pause espera el siguiente refresco o ctrl+c
    while (!salir)
    {
        if (refrescar)
        {
            struct timespec ahora;
            double intervalo;

            refrescar = 0;
            clock_gettime(CLOCK_MONOTONIC, &ahora);
            intervalo = segundos_transcurridos(&anterior, &ahora);
            notificar_background(lista, cantidad);
            dibujar(lista, cantidad, ticks_por_segundo, intervalo);
            anterior = ahora;
            alarm((unsigned int)segundos);
        }

        pause();
    }

    alarm(0);
    sigaction(SIGALRM, &anterior_alarm, NULL);
    sigaction(SIGINT, &anterior_int, NULL);
    printf("\n");
}
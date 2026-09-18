#include <signal.h>
#include <stdio.h>

#include "signals.h"

void configurar_senales_shell(void)
{
    struct sigaction accion = {0};

    // La shell ignora SIGINT (Ctrl+C) y SIGQUIT (Ctrl+\)
    // para evitar que el usuario cierre el proceso principal.
    accion.sa_handler = SIG_IGN;
    sigemptyset(&accion.sa_mask);

    // Reinicia llamadas al sistema que puedan ser interrumpidas
    // mientras la shell espera entrada u otra operación bloqueante.
    accion.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &accion, NULL) == -1)
    {
        perror("sigaction SIGINT");
    }

    if (sigaction(SIGQUIT, &accion, NULL) == -1)
    {
        perror("sigaction SIGQUIT");
    }
}

void restaurar_senales_hijo(void)
{
    struct sigaction accion = {0};

    // Los hijos heredan la configuración de señales del padre después
    // de fork(), por lo que aquí restauramos el comportamiento normal
    // antes de ejecutar el programa con execvp().
    accion.sa_handler = SIG_DFL;
    sigemptyset(&accion.sa_mask);
    accion.sa_flags = 0;

    if (sigaction(SIGINT, &accion, NULL) == -1)
    {
        perror("sigaction SIGINT");
    }

    if (sigaction(SIGQUIT, &accion, NULL) == -1)
    {
        perror("sigaction SIGQUIT");
    }
}
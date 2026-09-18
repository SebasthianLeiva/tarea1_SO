#ifndef SIGNALS_H
#define SIGNALS_H

// Configura SIGINT y SIGQUIT para que la shell los ignore.
void configurar_senales_shell(void);

// Restaura SIGINT y SIGQUIT al comportamiento por defecto en los hijos.
void restaurar_senales_hijo(void);

#endif
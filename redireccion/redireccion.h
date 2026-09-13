
#ifndef REDIRECCION_H
#define REDIRECCION_H



// redireccion de entrada y salida

void redireccionar_entrada_salida(char *archivo_salida, char *archivo_entrada, int append);


// permite eliminar los operadores (>,<,>>) de args luego de redireccionar, ya que
// execvp usara el mismo array de argumentos y dichos operadores no le sirven

void limpiar_redireccion_args(char *args[]);


// obtiene el archivo entrada y/o salida en caso de redireccion

void verificar_operadores_redir(int *append,char *args[], char **archivo_salida,
    char **archivo_entrada);



#endif


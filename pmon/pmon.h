
#include <stdio.h>

#include "../background/background.h"

//ejecuta pmon, imprimiendo comandos,pid,estado,cpu time y RSS, hasta recibir ctrl+c.
void pmon(struct Proceso_background lista_pros_background[],
          int cant_procesos_background,
          int segundos);


El programa requiere un compilador compatible con C11 y debe compilarse de la siguiente forma:

gcc -Wall -Wextra -std=gnu11 -o mishell main.c shell.c parsing/parser.c redireccion/redireccion.c background/background.c
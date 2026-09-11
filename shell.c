
#define _CRT_SECURE_NO_WARNINGS

#include "stdbool.h"
#include "stdio.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <string.h>



void shell() {

	while (true) {

		//imprime el directorio en el prompt 

		char directorio[1024];

        getcwd(directorio, sizeof(directorio));

        printf("%s ", directorio);


        char entrada_usuario[1024]; 


        //obtiene la entrada y la guarda en "entrada_usuario"

	    //si se oprime ctrl + D el programa termina

        if (fgets(entrada_usuario, sizeof(entrada_usuario), stdin) == NULL) {

            return;
            
        }


         //array para el programa y argumentos
        
		char* args[100];


        //se parsea la entrada y se insertan los tokens en el array

        char *token = strtok(entrada_usuario, " \n");

        int i = 0;

        while (token != NULL && i < 99) { 
        
        args[i] = token;

        i++;

        token = strtok(NULL, " \n");
        
         }

        args[i] = NULL;


        //si es que el usuario oprime "espacio" el while pasa a la siguiente iteracion
        //y la shell vuelve a pedir entrada
 
        if (args[0] == NULL) {
           continue;
        }

        //si el usuario escribe exit termina el programa

        if (strcmp(args[0], "exit") == 0) {

        return;

        }
 


		__pid_t pid = fork();
		
		if (pid == 0) {	//programa hijo

			execvp(args[0], args);
			perror("execvp"); /* solo si exec falla */
			_exit(127);
			
		}

		else if (pid > 0) { //programa del padre (shell)

			int status;

			waitpid(pid, &status, 0); //guarda el status en la variable
			
		}
			
		else { //el programa termina ante error fatal de fork
			
			perror("fork"); 

            return;
			
		}

	}
}




/*-----------------------------------------------------+      
 |     R E D I R E C C I O N . C                       |
 +-----------------------------------------------------+
 |     Asignatura :  SOP-GIIROB                        |
 |     Descripcion:                                    |
 +-----------------------------------------------------*/
#include "defines.h"
#include "redireccion.h"
#include "ejecucion.h"
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>


int ejecutar (int nordenes , int *nargs , char **ordenes , char ***args , int bgnd)
{
    int i;
    pid_t pid;
    int estado;
    
    // Los pipes se usan para conectar la salida estándar de un proceso con la entrada estándar de otro proceso.
    // Se necesitan nordenes - 1 pipes para conectar nordenes procesos en una tubería.
    // Cada pipe tiene 2 enteros, uno para leer y otro para escribir.
    int pipes[nordenes - 1][2];

    // Crear los pipes necesarios
    for (int i = 0; i < nordenes - 1; i++) {
        pipe(pipes[i]);
    }

    // Proseguimos a crear los procesos hijos
    for (i = 0; i < nordenes; i++) {
        // Creamos un nuevo proceso
        pid = fork();
        if (pid < 0) {
            perror("Error en el fork");
            return ERROR;
        }
        // Proceso hijo
        if (pid == 0) {
           if (i > 0) dup2(pipes[i - 1][0], STDIN_FILENO); // Redirigimos la entrada al pipe anterior
           if (i < nordenes - 1) dup2(pipes[i][1], STDOUT_FILENO); // Redirigimos la salida al pipe actual
        
            // Cerramos los pipelines ya que no los necesitamos en el padre
            for (int j = 0; j < nordenes - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // Ejecutamos la orden
            if (execvp(ordenes[i], args[i]) < 0) {
                // Si execvp falla, mostramos un error y salimos
                perror("Error en execvp");
                exit(EXIT_FAILURE);
            }
        }
    }
        // Aquí continua el proceso padre
        // Cerramos los pipes que ya no usa el padre, basicamente todos
        for (int i = 0; i < nordenes - 1; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }

        // Finalmente esperamos a que terminen los procesos hijos si no estamos en background
        if (!bgnd) {
            // Por lo que he leido, da igual si un hijo termina antes que el padre llegue a esta función, ya que el wait lo recoge aunque esté en estado de zombie
            // Así que simplemente hacemos un wait nordenes veces
            for (int i = 0; i < nordenes; i++) {
                wait(NULL);
            }
        }
        // Si estamos en background, no esperamos y devolvemos el control inmediatamente, init será el encargado de recoger los huerfanos si el padre termina antes, si no, solo quedan en estado zombie hasta que el padre termina
    return OK;
} 

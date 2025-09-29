/*-----------------------------------------------------+      
 |     R E D I R E C C I O N . C                       |
 +-----------------------------------------------------+
 |     Asignatura :  SOP-GIIROB                        |
 |     Descripcion:                                    |
 +-----------------------------------------------------*/
#include "defines.h"
#include "redireccion.h"
#include "ejecucion.h"
#include "profe.h"
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

// Función para verificar si es comando interno
int es_orden_interna(char *orden) {
    if (strcmp(orden, "cd") == 0) return 1;
    if (strcmp(orden, "exit") == 0) return 2;
    if (strcmp(orden, "pwd") == 0) return 3;
    if (strcmp(orden, "echo") == 0) return 4;
    return 0;
}


int ejecutar (int nordenes , int *nargs , char **ordenes , char ***args , int bgnd)
{
    int i;
    pid_t pid;
    
    // Manejamos si es un comando interno
    int caso = es_orden_interna(ordenes[0]);
    if (nordenes == 1 && caso != 0) {
        redirigir_entrada(0);
        redirigir_salida(0);
        switch (caso) {
            case 1: // cd
                if (nargs[0] < 2) {
                    // Usamos fprintf para enviar mensaje de error a stderr (esto ayuda a la realización de pruebas ya que separa la salida standard "stdout" de los errores "stderr")
                    fprintf(stderr, "El comando cd requiere de un argumento\n");
                    cerrar_fd();
                    return ERROR;
                }
                else {
                    // chdir cambia el directorio actual del proceso, que es lo que queremos para el comando cd
                    if (chdir(args[0][1]) != 0) {
                        // Usamos perror para mostrar el error específico que ha ocurrido, ya que chdir devuelve -1 en caso de error y cambia errno
                        perror("chdir");
                        cerrar_fd();
                        return ERROR;
                    }
                }
                break;
            case 2: // exit
                exit(0);
                break;
            case 3: // pwd
            // pwd muestra el directorio actual
                {
                    char cwd[1024];
                    // Obtenemos el directorio actual con getcwd
                    if (getcwd(cwd, sizeof(cwd)) != NULL) {
                        printf("%s\n", cwd);
                    } 
                    // Si hay error, mostramos un mensaje
                    else {
                        perror("getcwd");
                        cerrar_fd();
                        return ERROR;
                    }
                }
                break;
            case 4: // echo
                // echo imprime todos los argumentos separados por espacios
                for (int j = 1; j < nargs[0]; j++) {
                    printf("%s", args[0][j]);
                    if (j < nargs[0] - 1) {
                        printf(" ");
                    }
                }
                printf("\n");
                break;
        }
        cerrar_fd();
        return OK;
    }

    // Proseguimos a crear los procesos hijos
    for (int i = 0; i < nordenes; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // Hijo
            redirigir_entrada(i);
            redirigir_salida(i);
            cerrar_fd();
            execvp(ordenes[i], args[i]);
            perror("execvp");
            exit(ERROR);
        }
    }
    // El padre tras crear a todos los hijos cierra los pipes y espera a que terminen si no es background
    cerrar_fd();
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

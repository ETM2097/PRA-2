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
#include "analizador.h"
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "background_check.h"

// Función para verificar si es comando interno
int es_orden_interna(char *orden) {
    if (strcmp(orden, "cd") == 0) return 1;
    if (strcmp(orden, "exit") == 0) return 2;
    if (strcmp(orden, "pwd") == 0) return 3;
    if (strcmp(orden, "echo") == 0) return 4;
    return 0;
}

// He optado por no usar throw y catch para manejar errores, ya que en C no es común y complica el código innecesariamente
// En su lugar, las funciones devuelven códigos de error y se usan perror y fprintf para mostrar mensajes de error, perror muestra el mensaje asociado al valor de errno 
// por lo que es menos personalizable, mientras que fprintf permite mostrar mensajes personalizados en stderr

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
                    // Si no hay argumento, cambiamos al directorio home
                    const char *home = getenv("HOME");
                    if (home == NULL) {
                        // Si no se puede obtener el home, mostramos un error
                        // Usamos fprintf para mostrar el error en stderr
                        fprintf(stderr, "No se pudo obtener el directorio home\n");
                        cerrar_fd();
                        return ERROR;
                    }
                    if (chdir(home) != 0) {
                        perror("chdir");
                        cerrar_fd();
                        return ERROR;
                    }
                }
                /*
                else if (strcmp(args[0][1], "-") == 0) {
                    // Si el argumento es '-', cambiamos al directorio anterior
                    if (cd_prev_dir != NULL && *cd_prev_dir != NULL) {
                        if (chdir(*cd_prev_dir) != 0) {
                            perror("chdir");
                            cerrar_fd();
                            return ERROR;
                        }
                    }
                    else {
                        fprintf(stderr, "No hay directorio previo\n");
                        cerrar_fd();
                        return ERROR;
                    }
                }*/
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
    pid_t primer_pid = -1; // Guardamos el PID del primer proceso para el tracking de background
    // Proseguimos a crear los procesos hijos si hay más de una orden o no es un comando interno
    if (bgnd){
        pid_t padre = fork();
        if (padre > 0) return OK;
    }
    for (int i = 0; i < nordenes; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            // Error al crear el proceso
            perror("fork");
            cerrar_fd();
            return ERROR;
        }
        else if (pid == 0) {
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
    for (int i = 0; i < nordenes; i++) {
        wait(NULL);
    }

        // Si estamos en background, no esperamos y devolvemos el control inmediatamente, init será el encargado de recoger los huerfanos si el padre termina antes, si no, solo quedan en estado zombie hasta que el padre termina
    return OK;
} 

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

int check_env(char** cd_ant, int minus){
    // Esta función va a ser usada para modificar el vector de strings cd_ant, que contiene el directorio actual y el anterior
    // cd_ant[1] es el directorio actual y cd_ant[0] el anterior, conocemos como el actual al que se ha cambiado más recientemente, si no se ha cambiado nunca, estará vacío
    // Y cd_ant[0] sería el actual en el primer uso, que es el directorio inicial al ejecutar el shell
    // Nuestro cd, tendrá una opcion minus ('-') para cambiar al directorio anterior, es decir, hace un swap de posiciones y cambia al directorio anterior
    if (minus){
        // Primero checkamos que cd_ant[0] no esté vacío, de ser así, no hay directorio previo
        if (strcmp(cd_ant[0], "") == 0){
            fprintf(stderr, "No hay directorio previo\n");
            return ERROR;
        }
        else{
            // Guardamos el directorio donde estamos ahora (cd_ant[1]) en temp
            char temp[256];
            strcpy(temp, cd_ant[1]);
            // Cambiamos al directorio anterior (que está en cd_ant[0])
            if (chdir(cd_ant[0]) != 0) {
                perror("chdir");
                return ERROR;
            }
            // Actualizamos cd_ant[1] con el directorio al que acabamos de ir (cd_ant[0])
            strcpy(cd_ant[1], cd_ant[0]);
            // Y cd_ant[0] con el directorio donde estábamos (temp)
            strcpy(cd_ant[0], temp);
        }
    }
    else {
        // Si no es un swap, es decir, un cambio normal de directorio
        // Si es el primer uso, cd_ant[1] estará vacío, así que guardamos el directorio actual en cd_ant[1]
        if (strlen(cd_ant[1]) == 0){
            // Guardamos el directorio actual en cd_ant[1] si es el primer uso
            if (getcwd(cd_ant[1], 256) == NULL) {
                perror("getcwd");
                return ERROR;
            }
            return OK;
        }
        // Actualizamos cd_ant[0] con el directorio actual
        strcpy(cd_ant[0], cd_ant[1]);
        // Actualizamos cd_ant[1] con el directorio actual
        if (getcwd(cd_ant[1], 256) == NULL) {
            perror("getcwd");
            return ERROR;
        }
    }
    return OK;
}

// He optado por no usar throw y catch para manejar errores, ya que en C no es común y complica el código innecesariamente
// En su lugar, las funciones devuelven códigos de error y se usan perror y fprintf para mostrar mensajes de error, perror muestra el mensaje asociado al valor de errno 
// por lo que es menos personalizable, mientras que fprintf permite mostrar mensajes personalizados en stderr

int ejecutar (int nordenes , int *nargs , char **ordenes , char ***args , int bgnd, char **cd_ant)
{
    int i;
    pid_t pid;
    char cmds[nordenes][256];
    pid_t pids[nordenes]; // Array para almacenar los PIDs de los procesos hijos
    // Manejamos si es un comando interno
    int caso = es_orden_interna(ordenes[0]);
    if (nordenes == 1 && caso != 0) {
        redirigir_entrada(0);
        redirigir_salida(0);
        switch (caso) {
            case 1: // cd
                // Guardamos el directorio actual en cd_ant[0] antes de cambiarlo si es el primer uso
                if (strlen(cd_ant[0]) == 0){
                    // Guardamos el directorio actual en cd_ant[0] antes de cambiarlo
                    if (getcwd(cd_ant[0], 256) == NULL) {
                        perror("getcwd");
                        cerrar_fd();
                        return ERROR;
                    }
                }
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
                    if (check_env(cd_ant, 0) != OK) {
                        cerrar_fd();
                        return ERROR;
                    }
                }
                
                else if (strcmp(args[0][1], "-") == 0) {
                    // Si el argumento es '-', cambiamos al directorio anterior
                    if (check_env(cd_ant, 1) != OK) {
                        cerrar_fd();
                        return ERROR;
                    }
                }
                else {
                    // chdir cambia el directorio actual del proceso, que es lo que queremos para el comando cd
                    if (chdir(args[0][1]) != 0) {
                        // Usamos perror para mostrar el error específico que ha ocurrido, ya que chdir devuelve -1 en caso de error y cambia errno
                        perror("chdir");
                        cerrar_fd();
                        return ERROR;
                    }
                    if (check_env(cd_ant, 0) != OK) {
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
    if (bgnd){
        pid_t pid_f = fork();
        // Si hay segundo plano, los procesos solo deberán ignorar ciertas señales, SIGINT y SIGQUIT.
        // EL padre tiene ignoradas esas señales, y los hijos las restauran a default
        if (pid_f == 0){
            struct sigaction sa;
            sa.sa_handler = SIG_DFL;  // Establecer manejador como "default"
            sigemptyset(&sa.sa_mask);  // Máscara de señales vacía (no bloquear otras señales)
            sa.sa_flags = 0;  // Sin flags especiales
            sigaction(SIGINT, &sa, NULL);   
            sigaction(SIGQUIT, &sa, NULL); 
        } 
        if (pid_f < 0) {
            // Error al crear el proceso
            perror("fork");
            cerrar_fd();
            return ERROR;
        }
        else if (pid_f != 0){
            cerrar_fd();
            return OK; // El padre no espera al hijo
        }
    }
    // Proseguimos a crear los procesos hijos si hay más de una orden o no es un comando interno
    for (int i = 0; i < nordenes; i++) {
        pid_t pid = fork();
        if (!bgnd && pid == 0){
            // Restauramos las señales a default si el proceso no es en background
            struct sigaction sa;
            sa.sa_handler = SIG_DFL;  // Establecer manejador como "default"
            sigemptyset(&sa.sa_mask);  // Máscara de señales vacía (no bloquear otras señales)
            sa.sa_flags = 0;  // Sin flags especiales
            sigaction(SIGINT, &sa, NULL);   
            sigaction(SIGQUIT, &sa, NULL);
            sigaction(SIGTTIN, &sa, NULL);
            sigaction(SIGTTOU, &sa, NULL);
        }
        if (pid < 0) {
            // Error al crear el proceso
            perror("fork");
            cerrar_fd();
            return ERROR;
        }
        else if (pid == 0) {
            // Proceso hijo
            redirigir_entrada(i);
            redirigir_salida(i);
            cerrar_fd();
            execvp(ordenes[i], args[i]);
            perror("execvp");
            exit(ERROR);
        }
        else {
            // Proceso padre (supervisor)
            if (bgnd) {
                // Construimos la cadena del comando con sus argumentos
                char cmd[512];
                cmd[0] = '\0';
                strcat(cmd, ordenes[i]);
                for (int j = 1; j < nargs[i]; j++) {
                    strcat(cmd, " ");
                    if (args[i][j] != NULL) {
                        strcat(cmd, args[i][j]);
                    }
                }
                strcpy(cmds[i], cmd);
                pids[i] = pid;
            }
        }
    }
    
    // El padre (o supervisor) tras crear a todos los hijos cierra los pipes
    cerrar_fd();

    // Si es background, ha llegado aquí el proceso supervisor, que manejará la espera y mostrará los mensajes de terminación de sus hijos
        if (bgnd) {
            int status;
            int cont = 0;
            while(cont < nordenes){
                pid_t end = waitpid(pids[cont], &status, 0);
                
                if (end == pids[cont]) {
                    mostrar_terminacion(status, cmds[cont]);
                } else if (end == -1) {
                    perror("waitpid");
                }
                
                cont++;
            }
            exit(0);
        }
    // Si no es en background, el padre espera a que terminen todos los hijos
        else {
            for (int k = 0; k < nordenes; k++) {
                wait(NULL);
            }
        }   
    // Return OK si todo ha ido bien, solo debería llegar aquí el proceso padre
    return OK;
} 

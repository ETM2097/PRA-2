/*-----------------------------------------------------+      
 |     R E D I R E C C I O N . C                       |
 +-----------------------------------------------------+
 |     Version    :                        
 |     Autor :   
 |     Asignatura :  SOP-GIIROB                                                       
 |     Descripcion: 
 +-----------------------------------------------------*/
#include "defines.h"
#include "analizador.h"
#include "redireccion.h"
#include <unistd.h>

REDIRECCION_ORDENES red_ordenes;


void redireccion_ini(void)
{
    for (int i = 0; i < PIPELINE; ++i){
        red_ordenes[i].entrada = 0;
        red_ordenes[i].salida = 0;
    }
}//Inicializar los valores de la estructura cmdfd



int pipeline(int nordenes, char *infile, char *outfile, int append, int bgnd)
{
    if (nordenes > PIPELINE) {
        return ERROR;
    }

    if (nordenes < 1) {
        return ERROR;
    }

    // Inicializamos la estructura de redirección
    redireccion_ini();

    // Creamos pipes solo si hay más de un comando
    int pipes[nordenes - 1][2];
    if (nordenes > 1) {
        for (int i = 0; i < nordenes - 1; ++i) {
            if (pipe(pipes[i]) < 0) {
                perror("pipe");
                return ERROR;
            }
        }
    }

    for (int i = 0; i < nordenes; ++i) {
        // ENTRADA: Configurar entrada para el comando i
        if (i == 0) {
            // Primer comando: usar infile o stdin/dev/null, dependiendo si existe infile o si es en background
            if (strcmp(infile, "") != 0) {
                int fd_in = open(infile, O_RDONLY);
                if (fd_in < 0) {
                    perror("open infile");
                    return ERROR;
                }
                red_ordenes[i].entrada = fd_in;
            } 
            else if (bgnd == 1 && strcmp(infile, "") == 0) {
                int fd_in = open("/dev/null", O_RDONLY);
                if (fd_in < 0) {
                    perror("open /dev/null");
                    return ERROR;
                }
                red_ordenes[i].entrada = fd_in;
            }
            else if(strcmp(infile, "") != 0 && i == 0){
                // Si hay un archivo de entrada y es el primer comando
                int fd_in = open(infile, O_RDONLY);
                if (fd_in < 0) {
                    perror("open infile");
                    return ERROR;
                }
                red_ordenes[i].entrada = fd_in;
            } 
            else {
                red_ordenes[i].entrada = STDIN_FILENO;
            }
        } 
        else {
            // Comandos intermedios: usamos el pipe del comando anterior
            red_ordenes[i].entrada = pipes[i - 1][0];
        }

        // SALIDA: Configuramos la salida para el comando i
    if (i == nordenes - 1) {
            // Si es el último comando: usamos outfile o stdout/dev/null
            if (strcmp(outfile, "") != 0) {
                // HAY redirección explícita, usarla
                if (append) {
                    int fd_out = open(outfile, O_WRONLY | O_CREAT | O_APPEND, 0644);
                    if (fd_out < 0) {
                        perror("open outfile");
                        return ERROR;
                    }
                    red_ordenes[i].salida = fd_out;
                } else {
                    int fd_out = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd_out < 0) {
                        perror("open outfile");
                        return ERROR;
                    }
                    red_ordenes[i].salida = fd_out;
                }
            } 
            else {
                // NO hay redirección Y es foreground -> stdout
                red_ordenes[i].salida = STDOUT_FILENO;
            }
        } 
        else {
            // Comandos intermedios: usamos el pipe hacia el siguiente comando
            red_ordenes[i].salida = pipes[i][1];
        }
    }
    
    return OK;
} // Fin de la funci�n "pipeline"



int redirigir_entrada(int i)
{
    if (dup2(red_ordenes[i].entrada, STDIN_FILENO) < 0) {
        perror("dup2 entrada");
        return ERROR;
    }
    return OK;
} // Fin de la funci�n "redirigir_entrada"



int redirigir_salida(int i)
{
    if (dup2(red_ordenes[i].salida, STDOUT_FILENO) < 0) {
        perror("dup2 salida");
        return ERROR;
    }
    return OK;
} // Fin de la funci�n "redirigir_salida"


int cerrar_fd()
{
    // Cerramos todos los descriptores de archivo en red_ordenes
    // El array red_ordenes tiene tamaño PIPELINE, así que vamos de 0 a PIPELINE-1
    for (int i = 0; i < PIPELINE; ++i) {
        // Cerramos el descriptor de archivo de entrada si es mayor a 2
        if (red_ordenes[i].entrada > STDERR_FILENO) {
            close(red_ordenes[i].entrada);
            red_ordenes[i].entrada = 0;
        }

        // Cerramos el descriptor de archivo de salida si es mayor a 2
        if (red_ordenes[i].salida > STDERR_FILENO) {
            close(red_ordenes[i].salida);
            red_ordenes[i].salida = 0;
        }
    }
    return OK;
} // Fin de la funci�n "cerrar_fd"



#include "background_check.h"
#include <string.h>

// Dejo todo el código creado aquí, aunque finalmente solamente se vaya a usar la función mostrar_terminación y no vaya a utilizar la estructura de procesos en background
// La estructura y funciones para manejar procesos en background pueden ser útiles para futuras mejoras del shell
// Aunque de momento no se implementarlas para que al finalizar el background se impriman los mensajes de terminación
// Ya que verificar_procesos_bg() solo la puedo llamar en el proceso padre y se ejecuta en la siguiente iteración del bucle principal del shell
// Haciendo que los programas hijos en background no muestren el mensaje de terminación inmediatamente al finalizar
// Siendo zombies hasta que el padre los maneje, esto según he visto es debido a que al generar una copia de la estructura de procesos en background en el fork del hijo
// El hijo no puede modificar la estructura del padre, y al llamar el hijo a verificar_procesos_bg() no encuentra su PID en la estructura del padre
// Seguiré investigando este comportamiento para futuras versiones del shell

proceso_bg procesos_bg[MAX_BG_P];

void init_procesos_bg() {
    for (int i = 0; i < MAX_BG_P; i++) {
        procesos_bg[i].pid = -1; // Indica que la posición está libre
        procesos_bg[i].comando[0] = '\0';
        procesos_bg[i].status = 0;
    }
}

void agregar_proceso_bg(pid_t pid, const char* comando) {
    for (int i = 0; i < MAX_BG_P; i++) {
        if (procesos_bg[i].pid == -1) { // Encuentra una posición libre
            procesos_bg[i].pid = pid;
            strncpy(procesos_bg[i].comando, comando, 255); // Copiamos el comando marcando el buffer máximo
            procesos_bg[i].comando[255] = '\0'; // Aseguramos la terminación nula de la cadena.
            procesos_bg[i].status = 0; // Marcamos el proceso como en ejecución
            printf("[%d] %d %s\n", i + 1, pid, comando); // Mostrar: [número] PID comando
            return;
        }
    }
    fprintf(stderr, "No se pueden agregar más procesos en background, límite alcanzado: %d\n", MAX_BG_P); // Si hay demasiados procesos en BG, mostramos un error
}


void mostrar_terminacion(int status, const char* comando) {
    // La función WIFEXITED verifica si el proceso terminó normalmente (Mediante exit() o return)
    if (WIFEXITED(status)) {
        // WEXITSTATUS obtiene el código de salida del proceso
        int exit_status = WEXITSTATUS(status);
        if (exit_status == 0) {
            // Éxito POSIX: 0 significa correcto
            printf("DONE\nProceso '%s' terminado con éxito\n", comando);
        } else {
            // Código de error distinto de 0
            printf("EXIT %d\nProceso '%s' terminado con error\n", exit_status, comando);
        }
    // WIFSIGNALED verifica si el proceso terminó debido a una señal
    } else if (WIFSIGNALED(status)) {
        // WTERMSIG obtiene el número de la señal que causó la terminación
        printf("KILLED\nProceso '%s' terminado por señal %d\n", comando, WTERMSIG(status));
    }
}


void verificar_procesos_bg(int status, pid_t pid) {
        for (int i = 0; i < MAX_BG_P; i++) {
            if (procesos_bg[i].pid == pid) {
                mostrar_terminacion(status, procesos_bg[i].comando);
                procesos_bg[i].pid = -1; // Marca la posición como libre
                procesos_bg[i].comando[0] = '\0';
                procesos_bg[i].status = 0;
                break;
            }
        }
}
    

// Función no necesaria, pero la dejo ya que fué la primera versión junto a manejar zombies(aplicada en verificar_procesos_bg()) y sirve para eliminar un proceso en específico
// La función verificar_procesos_bg() es más eficiente ya que elimina cualquier proceso terminado y maneja zombies.
void eliminar_proceso_bg(pid_t pid) {
    for (int i = 0; i < MAX_BG_P; i++) {
        if (procesos_bg[i].pid == pid) {
            procesos_bg[i].pid = -1; // Marca la posición como libre
            procesos_bg[i].comando[0] = '\0';
            procesos_bg[i].status = 0;
            return;
        }
    }
}
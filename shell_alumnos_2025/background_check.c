#include "background_check.h"
#include <string.h>

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
        if (exit_status == 1) {
            printf("DONE\nProceso '%s' terminado con éxito\n", comando);
        } else {
            printf("EXIT %d\nProceso '%s' terminado con error\n", exit_status, comando);
        }
    // WIFSIGNALED verifica si el proceso terminó debido a una señal
    } else if (WIFSIGNALED(status)) {
        // WTERMSIG obtiene el número de la señal que causó la terminación
        printf("KILLED\nProceso '%s' terminado por señal %d\n", comando, WTERMSIG(status));
    }
}


void verificar_procesos_bg() {
    int status;
    for (int i = 0; i < MAX_BG_P; i++) {
        if (procesos_bg[i].pid != -1) { // Si la posición está ocupada (-1 significa libre)
            pid_t result = waitpid(procesos_bg[i].pid, &status, WNOHANG); // Verificamos el estado del proceso sin bloquear con WNOHANG
            if (result == -1) {
                perror("waitpid");
            } else if (result > 0) { // El proceso ha terminado y nos ha dado su estado
                mostrar_terminacion(status, procesos_bg[i].comando);
                procesos_bg[i].pid = -1; // Marca la posición como libre
                procesos_bg[i].comando[0] = '\0';
                procesos_bg[i].status = 0;
                // Eliminamos cualquier proceso zombie que haya quedado
            }
        }
    }
    manejar_zombies(); // Limpiamos cualquier zombie que haya quedado
}


// Función no necesaria, pero la dejo ya que fué la primera versión junto a manejar zombies y sirve para eliminar un proceso en específico
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

// Maneja procesos zombies que hayan terminado sin ser verificados.
void manejar_zombies() {
    while ((waitpid(-1, NULL, WNOHANG)) > 0) { // Usamos -1 para esperar a cualquier hijo (WNOHANG sirve para no bloquear el proceso).
        // Continuamos limpiando zombies hasta que no queden, ya que waitpid devuelve 0 si no hay zombies y -1 si hay error o no hay hijos
    }
}
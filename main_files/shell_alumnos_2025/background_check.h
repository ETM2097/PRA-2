#ifndef BACKGROUND_CHECK_H
#define BACKGROUND_CHECK_H

#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_BG_P 10

typedef struct {
    pid_t pid; // PID del proceso
    char comando[256]; // Comando del proceso
    int status; // Estado del proceso (0: en ejecución, 1: terminado, 2: error, 3: Terminado por señal)
} proceso_bg;

// Declaración externa del array de procesos, extern sirve para indicar que la variable se define en otro archivo
// y evitar múltiples definiciones al incluir este header en varios archivos
extern proceso_bg procesos_bg[MAX_BG_P];

void init_procesos_bg();
void verificar_procesos_bg();
void manejar_zombies();
void agregar_proceso_bg(pid_t pid, const char* comando);
void eliminar_proceso_bg(pid_t pid);
void mostrar_terminacion(int status, const char* comando);


#endif // BACKGROUND_CHECK_H
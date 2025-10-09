/*-----------------------------------------------------+
 |     U S H. C                                        
 +-----------------------------------------------------+
 |     Versión :                                       |                      
 |     Autor :                                         |
 |     Asignatura :  SOP-GIIROB                        |                               
 |     Descripción :                                   |
 +-----------------------------------------------------*/
#include "defines.h"
#include "analizador.h"
#include "redireccion.h"
#include "ejecucion.h"
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include "profe.h"
#include "visualizado.h"
#include "background_check.h"

#include <stdio.h>
#include <wchar.h>
#include <locale.h>



//
// Declaraciones de funciones locales
//
int leerLinea( char *linea, int maxLinea );
bool user_selection(void);
void set_signals(void);
//
// Prog. ppal.
// 
int main(int argc, char * argv[])
{
    
  char line[255];
  // Declaración de cd_ant como array de punteros, creamos dos buffers para almacenar los directorios
  char cd_ant_buf0[256] = "";
  char cd_ant_buf1[256] = "";
  char *cd_ant[2] = {cd_ant_buf0, cd_ant_buf1};
  int res;
  char **m_ordenes;
  char ***m_argumentos;
  int *m_num_arg;
  int m_n;

  
  bool selection = user_selection();

  while(1)
  {
    setlocale(LC_ALL, "");
    while(1)
    {
      set_signals();
      do
      {
          res=leerLinea(line,MAXLINE);    
          if (res==-2) {
        		fprintf(stdout,"logout\n");
        		exit(0);
          }
      	  if (res==-1){
        		fprintf(stdout,"linea muy larga\n");
        	  }
       }while(res<=0);

       if (analizar(line)==OK){
         m_n=num_ordenes();
         m_num_arg=num_argumentos();
         m_ordenes=get_ordenes();
         m_argumentos=get_argumentos();
         if(m_n>0)
         {
            // Guardar stdout original antes de redirecciones
            int stdout_backup = dup(STDOUT_FILENO);
            int stdin_backup = dup(STDIN_FILENO);
            
            if (pipeline(m_n,fich_entrada(),fich_salida(),es_append(),es_background())==OK)
                      ejecutar(m_n,m_num_arg,m_ordenes,m_argumentos,es_background(), cd_ant);
            
            // Restaurar stdout original para el prompt
            dup2(stdout_backup, STDOUT_FILENO);
            dup2(stdin_backup, STDIN_FILENO);
            close(stdout_backup);
            close(stdin_backup);
          }
        if (selection){
          visualizar_bonito();
        }
       }
    }    
  return 0;
}
}





/****************************************************************/
/*                       leerLinea                             
  --------------------------------------------------------------
                                                               
   DESCRIPCIÓN:                                                 
   Obtiene la línea de órdenes para el mShell.    
   Util para depuracion.                                        
                                                                
   ENTRADA:                                                 
    linea - puntero a un vector de carácteres donde se almancenan los caracteres 
   leídos del teclado
    maxLinea - tamaño máximo del vector anterior

   SALIDA:
    -- linea - si termina bien, contiene como último carácter el retorno de carro.
    -- leerLinea -  Entero que representa el motivo de finalización de la función:
     > 0 - terminación correcta, número de caracteres leídos, incluído '\n'
     -1 - termina por haber alcanzado el número máximo de caracteres que se 
    espera leer de teclado, sin encontrar '\n'.
     -2 - termina por haber leído fin de fichero (EOF).
*/
/****************************************************************/

// Esta función va a mostrar un PROMPT con el directorio actual mediante la función getcwd y despúés leer una línea de entrada (teclado) hasta encontrar
// un \n o EOF (Ctrl + D). Si la línea es demasiado larga, se vacía el buffer de entrada y se devuelve -1.
int leerLinea( char *linea, int maxLinea ){
  // Creamos un buffer para el directorio actual
  char cwd[1024];
  // Obtenemos el directorio home del usuario
  char* home = getenv("HOME");
  // Comprobamos que podemos obtener el directorio actual (getcwd devuelve NULL si hay error)
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
      // Mostramos el prompt por el terminal, con colores y sumamos a cwd la longitud de home para que muestre ~ en lugar del path completo
      printf("\033[1;32m%s\033[0m:~\033[1;34m%s\033[0m$ ", PROMPT, cwd + strlen(home)); 
  } 
  // Si hay error, mostramos un mensaje
  else {
      perror("getcwd() error");
  }
  // Creamos las variables i (índice del carácter actual) y c (carácter leído)
  int i = -1, c = 0;
  // Leemos caracteres hasta encontrar EOF, \n o alcanzar el máximo permitido
  while((c = getchar()) != EOF && c != '\n' && i < maxLinea - 1){
    // Guardamos el carácter leído en el buffer
    linea[++i] = c;
  } 
  // Si hemos llegado a EOF y no hemos leído nada, devolvemos -2
  if(c == EOF && i == -1){
    return -2; // Devolvemos -2 para indicar que ha habido un EOF
  }
  // Si hemos leído un \n, lo añadimos al buffer y devolvemos el número de caracteres leídos
  if(c == '\n'){
    linea[++i] = c;
    // Añadimos el carácter nulo al final de la cadena (IMPORTANTE)
    linea[++i] = '\0';
    return i; // Devuelve el número de caracteres leídos
  }
  // Si hemos alcanzado el máximo permitido sin encontrar \n, vaciamos el buffer y devolvemos -1
  if (i == maxLinea - 1) {
  // Vacía el buffer hasta el siguiente '\n' o EOF
  int ch;
  while ((ch = getchar()) != '\n' && ch != EOF);
  return -1; // Línea demasiado larga
  }
  // En caso de cualquier otro caso, devolvemos -1 para repetir la lectura
  return -1;
}

bool user_selection(void){
  bool selection = true;
  char input[3] = "";
  printf("Este programa es una versión de un Shell simple creado en prácticas de Sistemas Operativos.\n");
  printf("Cuenta con funcionalidades básicas como ejecución de comandos (internos y externos), tuberías, redirecciones, manejo de procesos en segundo plano y de señales.\n");
  printf("También cuenta con una opción de visualización de los comandos ejecutados en el shell y las redirecciones realizadas para poder observar las acciones realizadas.\n");
  printf("\033[1;32m¿Desea visualizar las órdenes ejecutadas y las redirecciones realizadas? (s/n): \033[0m");
  while(strcmp(input, "s\n") != 0 && strcmp(input, "n\n") != 0 && strcmp(input, "S\n") != 0 && strcmp(input, "N\n") != 0){
    // Usamos fgets para leer la entrada del usuario, debido a que nos permite controlar la entrada y evitar desbordamientos de buffer
    fgets(input, sizeof(input), stdin);
    if (strcmp(input, "s\n") == 0 || strcmp(input, "S\n") == 0){
      selection = true;
      break;
    }
    else if (strcmp(input, "n\n") == 0 || strcmp(input, "N\n") == 0){
      selection = false;
      break;
    }
    else{
      printf("Entrada no válida. Por favor, introduzca 's' para sí o 'n' para no: ");
    }
  
  }
  return selection;
}

void set_signals(void){
    // Ignoramos las señales SIGINT y SIGQUIT en el shell
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;  // Establecer el manejador como "ignorar"
    sigemptyset(&sa.sa_mask);  // Máscara de señales vacía (no bloquear otras señales)
    sa.sa_flags = 0;  // Sin flags especiales
    sigaction(SIGINT, &sa, NULL);   
    sigaction(SIGQUIT, &sa, NULL);
    // Ignoramos las señales SIGTTIN y SIGTTOU para evitar que el shell se detenga al intentar leer o escribir en la terminal
    sigaction(SIGTTIN, &sa, NULL);
    sigaction(SIGTTOU, &sa, NULL);
}
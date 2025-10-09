# define _XOPEN_SOURCE // Necesario para wcswidth() ya que no forma parte del estándar C
#include <stdio.h>
#include <wchar.h>
#include <locale.h>
#include "defines.h"
#include "analizador.h"
#include "profe.h"  

// Esta función print_aligned() toma una cadena de caracteres (contenido) y la imprime centrada dentro de un ancho fijo (WIDTH).
void print_aligned(const char *contenido) {
  // Usamos wcswidth para obtener el ancho visual de la cadena en caracteres anchos (ya que contiene caracteres Unicode)
  wchar_t wbuffer[512];
  // mbstowcs convierte una cadena multibyte (UTF-8) a una cadena wide character (wchar_t), mbstowcs(wchar_t *destino, const char *source, size_t tamanyo)
  mbstowcs(wbuffer, contenido, 512);
  // wcswidth calcula el ancho visual de la cadena wide character
  int display_len = wcswidth(wbuffer, 512);
  // Imprimimos la cadena centrada
  printf("|| %s", contenido);
  for (int i = 0; i < WIDTH - display_len - 4; i++) { // -4 por "|| " y "||"
    putchar(' '); // Rellenamos con espacios hasta completar el ancho
  }
  printf("||\n"); // Fianalmente cerramos la línea con "||"
}


// Visualizador_Bonito usa el mismo funcionamiento que visualizador() descrito al final del documento, pero usa de apoyo una función print_aligned() que centra el texto
// dentro de un ancho fijo (WIDTH) para dejar el texto encerrado en un recuadro.
void visualizar_bonito(void){
  printf("||==============================================================================================||\n");
  printf("||                                     \033[1;32mV\033[0m\033[1;34mI\033[0m\033[1;33mS\033[0m\033[1;35mU\033[0m\033[1;36mA\033[0m\033[1;31mL\033[0m\033[1;32mI\033[0m\033[1;33mZ\033[0m\033[1;34mA\033[0m\033[1;37mD\033[0m\033[1;32mO\033[0m\033[1;34mR\033[0m \033[1;33mE\033[0m\033[1;35mN\033[0m\033[1;36mR\033[0m\033[1;37mI\033[0m\033[1;31mC\033[0m                                       ||\n");
  printf("||==============================================================================================||\n");
  char buffer[1024];
  // snprintf escribe el texto formateado en el buffer
  snprintf(buffer, sizeof(buffer), "%s%s  Número de órdenes analizadas: %d  %s%s", "\U0001F916", "\U0001F4AC", num_ordenes(), "\U0001F916", "\U0001F4AC");
  // Pasamos el buffer a la función print_aligned para que lo imprima centrado
  print_aligned(buffer);
  print_aligned("");
  snprintf(buffer, sizeof(buffer), "🇦 🇷 🇬 🇺 🇲 🇪 🇳 🇹 🇴 🇸");
  print_aligned(buffer);
  print_aligned("");
  int cont = 1;
  for (int i = 0; i < num_ordenes(); i++) {
    for (int j = 0; j < num_argumentos()[i]; j++) {
      snprintf(buffer, sizeof(buffer), "%s Argumento %d %s %s", "\U0001F4D6", cont++, "\U0001F449", get_argumentos()[i][j]);
      print_aligned(buffer);
    }
  }
  print_aligned("");
  print_aligned("➡️➡️➡️  REDIRECCIONES  ⬅️⬅️⬅️");
  print_aligned("");
  if (fich_entrada() != NULL && fich_entrada()[0] != '\0') {
    snprintf(buffer, sizeof(buffer), "%s La redirección de entrada es -> %s %s", "\U0001F4E4", fich_entrada(), "\U0001F4E4");
    print_aligned(buffer);
  } else {
    snprintf(buffer, sizeof(buffer), "%s No hay redirección de entrada %s", "❌", "❌");
    print_aligned(buffer);
  }
  if (fich_salida() != NULL && fich_salida()[0] != '\0') {
    snprintf(buffer, sizeof(buffer), "%s La redirección de salida es -> %s %s", "\U0001F4E5", fich_salida(), "\U0001F4E5");
    print_aligned(buffer);
  } else {
    snprintf(buffer, sizeof(buffer), "%s No hay redirección de salida  %s", "❌", "❌");
    print_aligned(buffer);
  }
  print_aligned("");
  if (es_background()) {
    snprintf(buffer, sizeof(buffer), "%s La orden es en background %s", "\U0001F680", "\U0001F680");
    print_aligned(buffer);
  } else {
    snprintf(buffer, sizeof(buffer), "%s La orden es en foreground %s", "\U0001F6A7", "\U0001F6A7");
    print_aligned(buffer);
  }
  print_aligned("");
  printf("||==============================================================================================||\n");
}

#include "Entrada_c.h"

/* Función: imprime_ayuda
   Entrada: Cadena de caracteres
   Salida: Nula
   Descripcion: Imprime en la linea de comando ayuda sobre el uso del programa
*/
void imprime_ayuda(char* nombre) {
  printf("Uso: %s -d z -p y [-h]\n", nombre);
  printf("Descripción: programa servidor SVR.\n");
  printf("    -h   	Mostrar informacion sobre uso\n"
	 "    -p y 	Nro de puerto (z>1024 y que no que no este cerrado)\n"
	 "    -d z 	Direccion del servidor (svr_s)\n");
}
 
/* Función: Parametros
   Entrada: *nro_jugadores, *nro_tiradas,*semilla, argc: Nro de parametros de Main,  **argv: Arreglo de entrada de Main
   Salida: *nro_jugadores, *nro_tiradas,*semilla. TRUE: Exito. Errores: Finaliza el programa
   Descripcion: Procesa el pasaje de parametros por linea de c


omandos según la convencion de los sistemas Linux
*/ 
int Parametros(char *serv, int *nroPuerto, int argc, char **argv) {
  
  int opciones;
  // Parametros con : tienen argumentos obligatorios. Con :: son opcionales. Sin ellos no requieren argumentos
  char *parametros = "d:p:h";
  
  if (argc != 5) {
    imprime_ayuda(argv[0]);
    //printf("Nro valores: %d\n", argc);
    return FALSE;
  }
  
  while (-1!=(opciones=getopt(argc, argv, parametros))) {
    switch (opciones) {
      // "-l i" indica el numero de tiradas (i) que hara cada jugador.
    case 'p' :
      *nroPuerto=atoi(optarg);
      continue;
      /* "-s x", el argumento x es un entero. La semilla que utiliza croupier */
    case 'd' :
      strncpy(serv,optarg,strlen(optarg));
      continue;   
      // Parametros desconocidos			   
    case '?' : 
      printf("Opciones o valores invalidos\n");
      return FALSE;
      // Imprime ayuda
    case 'h' :
      imprime_ayuda(argv[0]);
      return FALSE;
      // Parametros no reconocidos
    default : 
      printf("Argumentos desconocidos. Use la opción -h para más informacion\n");
      return FALSE;
    }

  }
  if (serv==NULL || nroPuerto==0) {
    perror("Debe insertar valores válidos\n");
    return FALSE;
  } 
  return TRUE;
}
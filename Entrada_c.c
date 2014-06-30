/**
  * Implementacion de los parámetros de entrada de svr_c
  *
  * @file    Entrada_c.c
  * @author  Luiscarlo Rivera 09-11020, Daniel Leones 09-1097
  *
  */
#include "Entrada_c.h"

/**
 * Imprime ayuda para la ejecución
 * @param  nombre Nombre del programa
 */
void imprime_ayuda(char* nombre) {
  printf("Uso: %s -d z -p y [-h]\n", nombre);
  printf("Descripción: programa servidor SVR.\n");
  printf("    -h   	Mostrar informacion sobre uso\n"
	 "    -p y 	Nro de puerto (z>1024 y que no que no este cerrado)\n"
	 "    -d z 	Direccion del servidor (svr_s)\n");
}
 
/**
 * Filtra los parametros del programa dado los flags -d -p -h
 * @param  serv String vacio que guardara el nombre del servidor
 * @param  nroPuerto int no inicializado que contendra el numero de puerto
 * @param  argc Número de parámetros del programa invocador
 * @param  argv Parámetros del programa invocador
 * @return 0 e imprime ayuda si falló, 1 si tuvo exito
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

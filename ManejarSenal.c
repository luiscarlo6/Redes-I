#include "ManejarSenal.h"
/**
  * Manejo de señales para svr_s
  *
  * @file    ManejarSenal.c
  * @author  Luiscarlo Rivera 09-11020, Daniel Leones 09-10977
  *
  */

void (*oldHandler)();

/*Manejador de señal SIGPIPE*/
void manejador1(){
  signal(SIGPIPE, manejador1);  
  printf("\n\nConexion Perdida\n\n");
}


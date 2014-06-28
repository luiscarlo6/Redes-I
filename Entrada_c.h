#ifndef __ENTRADA_C_H__
#define __ENTRADA_C_H__

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FALSE 0
#define TRUE !FALSE

int Parametros(char *serv, int *nroPuerto, int argc, char **argv);

#endif

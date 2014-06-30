#include "Entrada_c.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#define BUFFER_LEN 1024
#define MAXNOSERVIDOR 30
#define RECUPERA "recuperacion.txt"

#ifndef __OPERACIONESERV_H_H__
#define __OPERACIONESERV_H_H__

int enviar_mensaje(int nroPuerto, struct hostent *he, char *mensaje);

int Abrir_Socket(int nroPuerto, struct hostent *he);

char* Pedir_Memoria(int tam);

char* Dar_Formato(char *mensaje);

int recuperar(int nroPuerto, struct hostent *he);

#endif

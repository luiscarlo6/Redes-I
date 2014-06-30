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
#define TO "09-11020@ldc.usb.ve"
#define FROM "svr_c"
#define SUBJECT "Alerta"

#ifndef __OPERACIONESERV_H_H__
#define __OPERACIONESERV_H_H__

int enviar_mensaje(int nroPuerto, struct hostent *he, char *mensaje);

int Abrir_Socket(int nroPuerto, struct hostent *he);

char* Pedir_Memoria(int tam);

char* Dar_Formato(char *mensaje);

int recuperar(int nroPuerto, struct hostent *he);

int sendmail(const char *to, const char *from, const char *subject, const char *message);

#endif

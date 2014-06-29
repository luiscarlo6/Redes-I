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
#define BUFFER_LEN 1024

#ifndef __OPERACIONESERV_H_H__
#define __OPERACIONESERV_H_H__

static char *mensajesValidos[] = {"Communication Offline",
				  "Communication error",
				  "Mensaje Desconocido",
				  "Low Cash alert",
				  "Running Out of notes in cassette",
				  "empty",
				  "Service mode entered",
				  "Service mode left",
				  "device did not answer as expected",
				  "The protocol was cancelled",
				  "Low Paper warning",
				  "Printer Error",
				  "Paper-out condition",
				  "Modo Recuperacion",
				  "Fin Recuperacion" };


int Abrir_Socket(int nroPuerto, char *nameServer);

char* Pedir_Memoria(int tam);

char* Dar_Formato(char *mensaje);

#endif

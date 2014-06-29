#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#define SERVER_PORT 4321
#define BUFFER_LEN 1024
#define MAXMENSAJE 100
#define MROMENSAJERRORES 15
#define NROCAMPOS 5
#define FALSE 0
#define TRUE !FALSE
#define TO "09-11020@ldc.usb.ve"
#define FROM "svr_s"
#define SUBJECT "Alerta"
#ifndef __OPERACIONESERV_H_H__
#define __OPERACIONESERV_H_H__

/*Estructura para datos entrantes de los hilos*/
typedef struct {
  int sockCliente;
  FILE *bitacoraG;
  FILE *bitacoraA;
  struct sockaddr_in idCliente;
} Args;

//typedef Argu Args;	

/*Estructura para datos salientes de los hilos*/
typedef struct {
  int recuperacion;
  char **mensajeArreglo;
  char *mensaje;
  int nroMensajes;
  struct sockaddr_in idCliente;
} Msg;

static char *mensajesErrores[] = {"Communication Offline",
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

int Abrir_Socket(int nroPuerto);

void* Atender_Clientes(void* dat);

int asignarDatosEntrantes(Args *caj, int sockCli, 
			  struct sockaddr_in idCli, FILE *bitaG, FILE *bitaA);

int procesarMensaje(FILE *bitacoraA, FILE* bitacoraG, 
		    char* buffer, int numbytes, char* salida, int *recuperar);

Msg* modoRecuperacion(int *recuperar, Args *regCliente);

int destruirDatosEntrantes(Args *caj);

int destruirResumen(char** resu, int tam);

int destruirInformes(Msg *caj, int tam);

int compararMensajes(char *palabra, char *p);

int sendmail(const char *to, const char *from, const char *subject, const char *message);

#endif

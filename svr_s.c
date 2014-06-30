/**
  * Implementación del servidor svr_s para monitoreo de alarmas de 
  * ATMs.
  * @file    svr_s.c
  * @author  Luiscarlo Rivera 09-11020, Daniel Leones 09-10977
  */
#include "Operacionesvr_s.h"
#include "ManejarSenal.h"
#include "Entrada_s.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <pthread.h>
#include <signal.h>
#define NROMAX_HILOS 1000
#define MAXNOMARCHIVO 30
#define FALSE 0
#define TRUE !FALSE

int main(int argc, char *argv[]) {

  int sockfd, clienteSockfd; /* descriptor para el socket */
  struct sockaddr_in their_addr; /* direccion IP y numero de puerto del cliente */
  pthread_t *clienteHilos= NULL; /*Arreglo de identificadores de hilos*/
  Args *datosEntrantes=NULL;
  Msg **informes=NULL;
  FILE *bitacoraG=NULL;
  FILE *bitacoraA=NULL;
  char *archSalida=NULL;
  char nomBitacoraG[MAXNOMARCHIVO];
  char nomBitacoraA[MAXNOMARCHIVO];
  int nrocliente=0, nroPuerto=0, i=0, j=0, enServicio=TRUE, err=0;
  pthread_mutex_t candado = PTHREAD_MUTEX_INITIALIZER;

  /* addr_len contendra el taman~o de la estructura sockadd_in y numbytes el
   * numero de bytes recibidos */
  socklen_t addr_len=0;

  /*Manejador de conexiones perdidas*/
  signal(SIGPIPE,manejador1);

  /*Buffer para guardar el nombre del archivo de la bitacora*/
  if ((archSalida=(char*)malloc(sizeof(char)*MAXNOMARCHIVO))==NULL) {    
    fprintf(stderr, "No hay memoria disponible: %s\n", strerror(errno));
    return FALSE;
  }	

  /*Recepcion de parametros de entrada*/
  if (!(Parametros(archSalida,&nroPuerto,argc,argv))) {
    exit(-1);  
  }

  printf("Archivo de bitacora: %s\n", archSalida);
  printf("Numero de puerto: %d\n", nroPuerto); 

  sprintf(nomBitacoraG,"%s_General.txt", archSalida);
  sprintf(nomBitacoraA,"%s_Alarmas.txt", archSalida);

  if ((bitacoraG=fopen(nomBitacoraG,"a"))==NULL) {
    perror("fopen: bitacora general no creada");
    exit(2);
  }

  if ((bitacoraA=fopen(nomBitacoraA,"a"))==NULL) {
    perror("fopen: bitacora general no creada");
    exit(2);
  }

  /*Arreglo para identificadores de hilos */
  clienteHilos=(pthread_t*)malloc(sizeof(pthread_t)*NROMAX_HILOS);
  /*Arreglo para datos de entrada de hilos*/
  datosEntrantes=(Args*)malloc(sizeof(Args)*NROMAX_HILOS);
  /*Arreglo para datosde salida de hilos*/
  informes=(Msg**)malloc(sizeof(Msg*)*NROMAX_HILOS);

  if ((datosEntrantes==NULL) || (clienteHilos==NULL) || (informes==NULL)) {
    perror("Memoria insuficiente");
    exit(-1);
  }

  free(archSalida);

  sockfd=Abrir_Socket(nroPuerto);

  while (enServicio) {			
    addr_len = sizeof(their_addr);
    /*Se atienden las solicitudes de conexion de cada cliente */
    if ((clienteSockfd=accept(sockfd,(struct sockaddr *) &their_addr, &addr_len))== -1)	{
    	err=errno;
    	manejarClienteCaido(err,their_addr);
      perror("accept");
      /*exit(5);*/
    }
    /*Crear estructuras de parametros para los hilos*/		
    asignarDatosEntrantes(&datosEntrantes[nrocliente],
			  clienteSockfd,their_addr,bitacoraG,bitacoraA);
		
    if (pthread_create(&clienteHilos[nrocliente],NULL,
		       Atender_Clientes,(void *) &datosEntrantes[nrocliente])!=0) {
      perror("ERROR: No se pudo crear el hilo\n");
    }
		
    /*Retorno de los hilos. Regresa con la estructura para informar las alarmas*/
    nrocliente++;
    for (i = 0; i < nrocliente; i++) {
      if (0!=pthread_join(clienteHilos[i],(void **) &informes[i])) {
      	perror("pthread_join");
      	enServicio=FALSE;
      }
      /*No se puede usar destruirDatosEntrantes parece que destruye la posicion del arreglo*/
      destruirDatosEntrantes(&datosEntrantes[i]);
      if (informes[i]!=NULL){
	      if (informes[i]->recuperacion){
	      	for (j = 0; j < informes[i]->nroMensajes; j++) {
	      	  //printf("Mensaje transmitido al servidor: %s\n", informes[i]->mensajeArreglo[j]);
		  sendmail(TO,FROM,SUBJECT, informes[i]->mensajeArreglo[j]);
	      	}
	      } else {
		       //printf("Mensaje transmitido al servidor: %s\n", informes[i]->mensaje);
		sendmail(TO,FROM,SUBJECT, informes[i]->mensaje);
	      }    		
	      /*Enviar por correo las alarmas*/
	      destruirInformes(informes[i],informes[i]->nroMensajes);      
	    }
	    nrocliente--;
    }	
  }
  pthread_mutex_destroy(&candado);

  close(sockfd);
  close(clienteSockfd);
  free(clienteHilos);
  free(datosEntrantes);
  free(informes);		
  fclose(bitacoraG);
  fclose(bitacoraA);
  exit(0);
}

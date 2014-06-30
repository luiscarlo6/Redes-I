/**
  * Implementación de las funciones y constantes del programa svr_c
  *
  * @file    Operacionesvr_c.c
  * @author  Luiscarlo Rivera 09-11020, Daniel Leones 09-1097
  *
  */
#include "Operacionesvr_c.h"


/**
 * Envia un mensaje al servidor
 * @param  nroPuerto  Número del puerto del servidor
 * @param  he Nombre del servidor al que se enviara el mensaje
 * @param  mensaje Mensaje a enviar al servidor
 * @return 0 si se envió el mensaje, 1 si falló
 */
int enviar_mensaje(int nroPuerto,struct hostent *he,char *mensaje) {
  int sockfd; /* descriptor para el socket */
  int numbytes; /* conteo de bytes a escribir */
  struct sockaddr_in their_addr; /* direccion IP y numero de puerto local */

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    //    exit(2);
    return -1;
  }
  
  /* a donde mandar */
  their_addr.sin_family = AF_INET; /* usa host byte order */
  their_addr.sin_port = htons(nroPuerto); /* usa network byte order */
  their_addr.sin_addr = *((struct in_addr *)he->h_addr);
  bzero(&(their_addr.sin_zero), 8); /* pone en cero el resto */
  
  /* Ordenar conexión */
  if (connect(sockfd,(struct sockaddr *) &their_addr,sizeof(their_addr))){
    perror("Imposible conectarse");
    //    exit(3);
    return -1;
  }

  if ((numbytes=send(sockfd,mensaje,strlen(mensaje),0))==-1){
    perror("Error de envio");
    //    exit(4);
    return -1;
  }
  /* cierro socket */  
  close(sockfd);   
  return 0; 
}

/**
 * Abre un socket para enviar información
 * @param  nroPuerto  Número del puerto del servidor
 * @param  he Nombre del servidor al que se enviara el mensaje
 * @return Descriptor del socket si tuvo exito, -1 si falló
 */
int Abrir_Socket(int nroPuerto,struct hostent *he) {
  int sockfd; /* descriptor para el socket */
  struct sockaddr_in their_addr; /* direccion IP y numero de puerto local */

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    //    exit(2);
    return -1;
  }
  
  /* a donde mandar */
  their_addr.sin_family = AF_INET; /* usa host byte order */
  their_addr.sin_port = htons(nroPuerto); /* usa network byte order */
  their_addr.sin_addr = *((struct in_addr *)he->h_addr);
  bzero(&(their_addr.sin_zero), 8); /* pone en cero el resto */
  
  /* Ordenar conexión */
  if (connect(sockfd,(struct sockaddr *) &their_addr,sizeof(their_addr))){
    perror("Imposible conec tarse");
    //    exit(3);
    return -1;
  }
  return sockfd; 
}

char* Pedir_Memoria(int tam){
  char *str = NULL;
  if ((str=(char*)malloc(sizeof(char)*tam))==NULL) {    
    fprintf(stderr, "No hay memoria disponible: %s\n", strerror(errno));
    return FALSE;
  }
  return str;
}

/**
 * Formatea un string para ser soportado por svr_s
 * @param  mensaje Mensaje sin formato a envíar
 * @return el mensaje formateado 
 */
char* Dar_Formato(char *mensaje){
  char *msj = Pedir_Memoria(BUFFER_LEN);
  time_t tiempo = time(0);
  struct tm *tlocal = localtime(&tiempo);
  char output[128];
  strftime(output,128,"%H:%M %d/%m/%y",tlocal);
  sprintf(msj,"%d ATM %s %s", getpid(),output,mensaje);
  free(mensaje);
  return msj;
}

/**
 * Rutina de recuperación del svr_c
 * @param  nroPuerto puerto de conexion al servidor
 * @param  he Nombre del servidor al que se enviara el mensaje
 * @return 0 si tuvo exito -1 si no
 */
int recuperar(int nroPuerto, struct hostent *he){
  FILE *archRec = fopen(RECUPERA,"r");
  char *mensaje = Pedir_Memoria(BUFFER_LEN);
  while (feof(archRec)==0){    
    sleep(1);
    fgets(mensaje,BUFFER_LEN,archRec);
    mensaje[strlen(mensaje)-1]='\0';
    printf("%s\n",mensaje);
    if (enviar_mensaje(nroPuerto,he,mensaje)==-1){
      fclose(archRec);
      return -1;
    }
  }
  fclose(archRec);
  free(mensaje);
  return 0;
}

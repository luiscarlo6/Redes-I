/*
 * Ejemplo de cliente de chat simple con datagramas (UDP).
 *
 * Leandro Lucarella - Copyleft 2004
 * Basado en diversos ejemplos públicos.
 *
 */
#include "Operacionesvr_c.h"
int main(int argc, char *argv[])
{
  int nroPuerto=0;
  int sockfd; /* descriptor a usar con el socket */
  int numbytes; /* conteo de bytes a escribir */
  char *nameServer=NULL; /*Para guardar el nombre del servidor*/
  char *mensaje=NULL;    /*Para guardar el mensaje a enviar recibido por entrada estandar*/
 
  nameServer = Pedir_Memoria(MAXNOSERVIDOR);
  if (!nameServer){
    return FALSE;
  }
  
  if (!(Parametros(nameServer,&nroPuerto,argc,argv))) {
    exit(-1);  
  }
  
  mensaje = Pedir_Memoria(BUFFER_LEN);
  if (!mensaje){
    return FALSE;
  }
  memset(mensaje, 0, sizeof(mensaje));
  while(TRUE){
    sleep(1);
    fflush(stdin);
    scanf("%[^\n]",mensaje);
    getchar();
    if (strncmp(mensaje,"",1)==0){
      break;
    }
    mensaje = Dar_Formato(mensaje);
    printf("%s\n",mensaje);

    /* Creamos el socket */
    sockfd=Abrir_Socket(nroPuerto,nameServer);

    if ((numbytes=send(sockfd,mensaje,strlen(mensaje),0))==-1){
      perror("Error de envio");
      exit(4);
    }
    /* cierro socket */  
    close(sockfd);   
  }

  free(mensaje);
  free(nameServer);
  exit (0);
}


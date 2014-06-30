/**
  * Implementación del cliente de un cajero automatico que se conecta con el servidor svr_s
  *
  * @file    svr_c.c
  * @author  Luiscarlo Rivera 09-11020, Daniel Leones 09-1097
  *
  */
#include "Operacionesvr_c.h"
int main(int argc, char *argv[])
{
  int sockfd;
  int errConex;
  int inicio;
  int nroPuerto=0;
  struct hostent *he;
  int enRecuperacion = FALSE;
  char *mensajeRecuperacion;
  FILE *archRec;
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

  mensajeRecuperacion = Pedir_Memoria(BUFFER_LEN);
  if (!mensajeRecuperacion){
    return FALSE;
  }
  memset(mensaje, 0, sizeof(mensaje));

    /* convertimos el hostname a su direccion IP */
  if ((he=gethostbyname(nameServer)) == NULL) {
    perror("gethostbyname");
    //    exit(1);
    return -1;
  } 
  
  archRec = fopen(RECUPERA,"r");
  if (archRec != NULL){
    fclose(archRec);
    recuperar(nroPuerto, he);
    system("rm recuperacion.txt");
  }

  
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
    
    if (!enRecuperacion){
      errConex = enviar_mensaje(nroPuerto,he,mensaje);
      if (errConex == -1){
	inicio = 0;
	enRecuperacion = TRUE;
	archRec = fopen(RECUPERA,"a");
	fprintf(archRec,"%s\n",mensaje);
	continue;
      }
    }
    else{
      fprintf(archRec,"%s\n",mensaje);
      sockfd = Abrir_Socket(nroPuerto,he);
      if (sockfd!=-1){
	close(sockfd);
	fclose(archRec);
	if (recuperar(nroPuerto, he)==-1)
	  continue;
	system("rm recuperacion.txt");
	enRecuperacion=FALSE;
	continue;
      }
      inicio++;
      if (inicio >= 300){
	fclose(archRec);
	sendmail(TO,FROM,SUBJECT,Dar_Formato("Servidor no disponible"));
	break;
      }
    }
  }
  free(mensajeRecuperacion);
  free(mensaje);
  free(nameServer);
  exit (0);
}



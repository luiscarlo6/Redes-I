#include "Operacionesvr_c.h"

int Abrir_Socket(int nroPuerto,char *nameServer) {
  int sockfd; /* descriptor para el socket */
  struct sockaddr_in their_addr; /* direccion IP y numero de puerto local */
  struct hostent *he; /* para obtener nombre del host */

  /* convertimos el hostname a su direccion IP */
  if ((he=gethostbyname(nameServer)) == NULL) {
    perror("gethostbyname");
    exit(1);
  }
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    exit(2);
  }
  
  /* a donde mandar */
  their_addr.sin_family = AF_INET; /* usa host byte order */
  their_addr.sin_port = htons(nroPuerto); /* usa network byte order */
  their_addr.sin_addr = *((struct in_addr *)he->h_addr);
  bzero(&(their_addr.sin_zero), 8); /* pone en cero el resto */
  
  /* Ordenar conexión */
  if (connect(sockfd,(struct sockaddr *) &their_addr,sizeof(their_addr))){
    perror("Imposible conectarse");
    exit(3);
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

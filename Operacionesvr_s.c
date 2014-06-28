#include "Operacionesvr_s.h"

pthread_mutex_t candado;

int Abrir_Socket(int nroPuerto) {
  int sockfd; /* descriptor para el socket */
  struct sockaddr_in my_addr; /* direccion IP y numero de puerto local */

  /* se crea el socket */
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket no creado");
    exit(1);
  }

  /* Se establece la estructura my_addr para luego llamar a bind() */
  my_addr.sin_family = AF_INET; /* usa host byte order */
  my_addr.sin_port = htons(nroPuerto); /* usa network byte order */
  my_addr.sin_addr.s_addr = INADDR_ANY; /* escuchamos en todas las IPs */
  bzero(&(my_addr.sin_zero), 8); /* rellena con ceros el resto de la estructura */

  /* Se le da un nombre al socket (se lo asocia al puerto e IPs) */
  printf("Asignado direccion al socket ....\n");
  if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
    perror("bind");
    exit(3);
  }

  /*Se ordena la escucha para atender las solicitudes de conexión */
  if (listen(sockfd,5)==-1) {
    perror("listen");
    exit(4);
  }

  return sockfd;
}

/*Funcion para atender las peticiones de los clientes a través de hilos*/
/*./ClienteChat localhost "03 ATM 10:04 22/07/14 Paper-out condition"*/
/*./ClienteChat localhost "03 ATM 10:04 22/07/14 empty"*/
/*./ClienteChat localhost "03 ATM 10:04 22/07/14 HolaMundo"*/
void* Atender_Clientes(void* dat)
{	
  Args *registroCliente = (Args *) dat; 
  Msg *informacion=NULL; 
  int numbytes=0;

  char *buffer=NULL; 
  char *bufferCopia=NULL; 
  char *buffer2=NULL; 
  char *subcadena=NULL; 
  char *cadena=NULL; 	
  char *terminal="\0"; 	
  char *entradaBita=NULL; 
  int k=0,i=0, cent=TRUE;

  /* Buffer de recepción */
  if ((buffer=(char*)malloc(sizeof(char)*BUFFER_LEN))==NULL) {    
    fprintf(stderr, "No hay memoria disponible: %s\n", strerror(errno));
  }
  memset(buffer, 0, sizeof(buffer));

  /* Recepción de datos */
  /*printf("sockClienteHilo: %d\n", registroCliente->sockCliente);*/
  if ((numbytes=recv(registroCliente->sockCliente,buffer,BUFFER_LEN,0))==-1)	{
    perror("recv");
    exit(6);
  }

  if ((bufferCopia=(char*)malloc(sizeof(char)*BUFFER_LEN))==NULL) {  
    fprintf(stderr, "No hay memoria disponible: %s\n", strerror(errno));
  }	
  memset(bufferCopia, 0, sizeof(bufferCopia));

  /*buffer[numbytes+1]="\0";*/
  strncat(buffer,terminal,2);
  /* Se visualiza lo recibido y se da la orden de escritura en la bitacora */
  printf("\npaquete proveniente de : %s\n",inet_ntoa((registroCliente->idCliente).sin_addr));
  printf("longitud del paquete en bytes: %d\n",numbytes);	
  printf("el paquete contiene: %s %d\n", buffer, (int)strlen(buffer));

  if ((entradaBita=(char*)malloc(sizeof(char)*(numbytes+1)))==NULL) {    
    fprintf(stderr, "No hay memoria disponible: %s\n", strerror(errno));
  }

  memset(entradaBita, 0, sizeof(entradaBita));
  strncpy(bufferCopia,buffer,numbytes+1);

  for (k = 0, cadena=buffer; k<(NROCAMPOS-1) ; k++, cadena=NULL) {
    if (NULL!=(subcadena=strtok_r(cadena, " ", &buffer2))) {
      strncat(entradaBita,subcadena, strlen(subcadena)+1);
      strncat(entradaBita," ",2);		
      printf("subcadena: %s %d\n", subcadena, (int)strlen(subcadena));
      /*printf("entrada a bitacora(Antes): (%s)\n", entradaBita);*/
    }
  }

  strncat(buffer2,terminal,3);
  for (i = 0; i < MROMENSAJERRORES ; i++)	{
    if (compararMensajes(mensajesErrores[i],buffer2)) {
      printf("Vigia\n");
      strncat(entradaBita,buffer2,strlen(buffer2)+1);
      pthread_mutex_lock(&candado);	
      fprintf(registroCliente->bitacoraA, "%s\n", entradaBita);
      fflush(registroCliente->bitacoraA);
      pthread_mutex_unlock(&candado);
      cent=FALSE;
    }
  }
  if (cent){
    buffer2=NULL;
    subcadena=NULL;
    cadena=NULL;
    printf("entrada a bitacora(No Error): (%s)\n", entradaBita);
    strncat(entradaBita,buffer2,strlen(buffer2));
    pthread_mutex_lock(&candado);	
    fprintf(registroCliente->bitacoraG, "%s\n", entradaBita);
    fflush(registroCliente->bitacoraG);
    pthread_mutex_unlock(&candado);
  }
  printf("entrada a bitacora: (%s) %d -- %d\n", entradaBita, (int)strlen(entradaBita), numbytes);	

  /*Esto fuerza el buffer de salida del sistema a escribir. Creo que para prueba de escritura será necesario usarlo. 
    Pero cuando se con los 1000 hilos se deberia quitar para mejorar la eficiencia dado que la escritura es en nivel
    privilegiado. Si no se usa el sistema no escribe en la bitacora */
  /*http://stackoverflow.com/questions/16780908/understanding-the-need-of-fflush-and-problems-associated-with-it*/
  /*fflush(registroCliente->bitacora);*/

  /*Crear estructura de retorno al programa principal */
  if ((informacion=(Msg*)malloc(sizeof(Msg)))==NULL) {	
    fprintf(stderr, "Datos sin retornar: %s\n", strerror(errno));
  }

  if ((informacion->mensaje=(char*)malloc(sizeof(char)*(numbytes+1)))==NULL) {    
    fprintf(stderr, "Datos sin retornar: %s\n", strerror(errno));
  }

  strncpy(informacion->mensaje,bufferCopia,numbytes+1);	
  informacion->idCliente=registroCliente->idCliente;

  free(buffer);
  free(bufferCopia);
  free(entradaBita);
		
  pthread_exit(informacion); 
}

int asignarDatosEntrantes(Args *caj, int sockCli, struct sockaddr_in idCli, FILE *bitaG, FILE *bitaA){
  
  if (caj == NULL) 
    return FALSE;

  caj->sockCliente=sockCli;
  caj->idCliente=idCli;
  caj->bitacoraA=bitaA;
  caj->bitacoraG=bitaG;
  return TRUE;
}

int destruirDatosEntrantes(Args *caj){
  
  if (caj == NULL) 
    return FALSE;
   
  close(caj->sockCliente);
  caj->bitacoraG=NULL;
  caj->bitacoraA=NULL;
  return TRUE;
}

int destruirInformes(Msg *caj){
  
  if (caj == NULL) 
    return FALSE;
    
  free(caj->mensaje);
  free(caj);
  return TRUE;
}

int compararMensajes(char *palabra, char *p) {
    
  int i=0;
  if (palabra == NULL)
    return 2;
    
  i=strncasecmp(palabra,p,strlen(palabra));
    
  //printf("%s ",caj1->palabra);
  //printf("%s ",p);
  //printf("%d\n",i);
    
  if (i != 0)
    return FALSE;
  else 
    return TRUE;
}

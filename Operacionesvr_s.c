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
/*./ClienteChat localhost "03_ATM_10:04_22/07/14_Paper-outCondition"*/
/*./ClienteChat localhost "03 ATM 10:04 22/07/14 empty"*/
/*./ClienteChat localhost "03 ATM 10:04 22/07/14 Modo Recuperacion"*/
/*./ClienteChat localhost "03 ATM 10:04 22/07/14 HolaMundo"*/


void* Atender_Clientes(void* dat)
{	
  Args *registroCliente = (Args *) dat; 
  Msg *informacion=NULL; 
  int numbytes=0;
  char *buffer=NULL;
  char* alarma=NULL;
  int recuperacion=FALSE;

  /* Buffer de recepción */
  if ((buffer=(char*)malloc(sizeof(char)*BUFFER_LEN))==NULL) {    
    fprintf(stderr, "No hay memoria disponible: %s\n", strerror(errno));
    pthread_exit(NULL); 
  }
  memset(buffer, 0, sizeof(buffer));

  /* Recepción de datos */
  if ((numbytes=recv(registroCliente->sockCliente,buffer,BUFFER_LEN,0))==-1)	{
    perror("recv");
    pthread_exit(NULL); 
  }
  if ((alarma=(char*)malloc(sizeof(char)*numbytes))==NULL) {    
    fprintf(stderr, "No hay memoria disponible: %s\n", strerror(errno));
    pthread_exit(NULL); 
  }	
  memset(alarma, 0, sizeof(alarma));

  buffer[numbytes]='\0';
  /* Se visualiza lo recibido y se da la orden de escritura en la bitacora */
  printf("\npaquete proveniente de : %s\n",inet_ntoa((registroCliente->idCliente).sin_addr));
  printf("longitud del paquete en bytes: %d\n",numbytes);	
  printf("el paquete contiene: %s\n", buffer);

  if (!(procesarMensaje(registroCliente->bitacoraA, registroCliente->bitacoraG,
			buffer,numbytes,alarma,&recuperacion))) {
    pthread_exit(NULL); 
  }

  /*Esto fuerza el buffer de salida del sistema a escribir. Creo que para prueba de escritura será necesario usarlo. 
    Pero cuando se con los 1000 hilos se deberia quitar para mejorar la eficiencia dado que la escritura es en nivel
    privilegiado. Si no se usa el sistema no escribe en la bitacora */
  /*http://stackoverflow.com/questions/16780908/understanding-the-need-of-fflush-and-problems-associated-with-it*/
  /*fflush(registroCliente->bitacora);*/

  if (recuperacion){
    informacion=modoRecuperacion(&recuperacion,registroCliente);
  } else {
    /*Modo normal*/
    /*Crear estructura de retorno al programa principal */
    if ((informacion=(Msg*)malloc(sizeof(Msg)))==NULL) {	
      fprintf(stderr, "Datos sin retornar: %s\n", strerror(errno));
      pthread_exit(NULL); 
    }

    if ((informacion->mensaje=(char*)malloc(sizeof(char)*(numbytes+1)))==NULL) {    
      fprintf(stderr, "Datos sin retornar: %s\n", strerror(errno));
      pthread_exit(NULL); 
    }
    memset(informacion->mensaje, 0, sizeof(informacion->mensaje));
    alarma[strlen(alarma)]='\0';
    strncpy(informacion->mensaje,alarma,strlen(alarma)+1);

    informacion->recuperacion=FALSE;
    informacion->idCliente=registroCliente->idCliente;
    informacion->mensajeArreglo=NULL;
    informacion->nroMensajes=1;

    free(buffer);
    free(alarma);
  }
  pthread_exit(informacion); 
}

int procesarMensaje(FILE *bitacoraA, FILE* bitacoraG, 
		    char* buffer, int numbytes, char* salida, int *recuperar){

  char *buffer2=NULL; 
  char *subcadena=NULL; 
  char *cadena=NULL;
  char* entradaBita=NULL; 	
	
  int k=0,i=0, cent=TRUE;

  if ((buffer==NULL) || (numbytes>0)) {
    printf("resumen: %s\n", buffer);
    if ((entradaBita=(char*)malloc(sizeof(char)*(numbytes+1)))==NULL) {    
      fprintf(stderr, "No hay memoria disponible: %s\n", strerror(errno));
      salida=NULL;
      return FALSE;
    }  	
    memset(entradaBita, 0, sizeof(entradaBita));

    /* Disecciona los campos del formato hasta encontrar la descripcion del mensaje. */
    for (k = 0, cadena=buffer; k<(NROCAMPOS-1) ; k++, cadena=NULL) {
      if (NULL!=(subcadena=strtok_r(cadena, " ", &buffer2))) {
	strncat(entradaBita,subcadena, strlen(subcadena));
	strncat(entradaBita," ",2);		
	printf("subcadena: %s\n", subcadena);
      }
    }

    /*	
     *	Clasificacion de los patrones de errores y escritura en la
     *	bitacora de alarmas. Si no encuentra un error pasa
     *	escribir en la bitacora general. Se pasa la descripcion del
     *	mensaje a Atender hilos.
     */
    buffer2[strlen(buffer2)]='\0';
    for (i = 0; i < MROMENSAJERRORES ; i++)	{
      if (compararMensajes(mensajesErrores[i],buffer2)) {				
	strncat(entradaBita,buffer2,strlen(buffer2)+1);
	strncpy(salida,buffer2,strlen(buffer2)+1);
	pthread_mutex_lock(&candado);	
	fprintf(bitacoraA, "%s\n", entradaBita);
	fflush(bitacoraA);
	sendmail(TO,FROM,SUBJECT, entradaBita);
	pthread_mutex_unlock(&candado);
	cent=FALSE;
      }
    }

    /*Activa el modo de recuperacion pasando un booleano hasta el main del hilo.*/
    if (compararMensajes(mensajesErrores[MROMENSAJERRORES-2],buffer2)) {
      *recuperar=TRUE;
    }

    /*Escritura en la bitacora general. */
    if (cent){			
      printf("entrada a bitacora(No Error): (%s)\n", entradaBita);
      strncat(entradaBita,buffer2,strlen(buffer2)); 
      pthread_mutex_lock(&candado);	
      fprintf(bitacoraG, "%s\n", entradaBita);
      fflush(bitacoraG);
      pthread_mutex_unlock(&candado);
    }

    printf("entrada Final a bitacora: (%s) %d -- %d\n", entradaBita, (int)strlen(entradaBita), numbytes);	
    free(entradaBita);
    subcadena=NULL;
    cadena=NULL;		
  }
  return TRUE;
}

/*	03 ATM 10:04 22/07/14 HolaMundo
	05 ATM 10:04 22/07/14 Printer Error
	06 ATM 10:04 22/07/14 Service mode left
	04 ATM 10:04 22/07/14 Modo Recuperacion
*/

/* Para hacer la recuperacion:
   -Se envia el mensaje Modo Recuperacion
   -Se envia la cantidad de mensajes perdidos 

   Mantener el socket del cliente abierto*/

/*Procedimiento para ejecutar el modo recuperacion del servidor. */
Msg* modoRecuperacion(int *recuperar, Args *regCliente){
	
  int numbytes=0;
  char* buffer=NULL; 
  char** resumen=NULL;
  char** salida=NULL;	
  Msg *informacion=NULL; 
  int i=0, nroMensajes=0;
  printf("MODO DE RECUPERACION\n");

  /*Buffer de recepción */
  if ((buffer=(char*)malloc(sizeof(char)*BUFFER_LEN))==NULL) {    
    fprintf(stderr, "No hay memoria disponible: %s\n", strerror(errno));
    pthread_exit(NULL); 
  }
  memset(buffer, 0, sizeof(buffer));

  /*Crear estructura de retorno al programa principal */
  if ((informacion=(Msg*)malloc(sizeof(Msg)))==NULL) {	
    fprintf(stderr, "Datos sin retornar: %s\n", strerror(errno));
    pthread_exit(NULL); 
  }

  /*Recepcion del numero de entradas perdidas*/
  if ((numbytes=recv(regCliente->sockCliente,buffer,BUFFER_LEN,0))==-1)	{
    perror("recv");
    pthread_exit(NULL); 
  }

  buffer[numbytes]='\0';
  printf("Nro mensajes: %s\n", buffer);
  nroMensajes=atoi(buffer);	
  /*Arreglo de entradas perdidas*/	
  if ((resumen=(char**)malloc(sizeof(char*)*nroMensajes))==NULL) {    
    fprintf(stderr, "Datos sin retornar: %s\n", strerror(errno));
    pthread_exit(NULL); 
  }
  memset(resumen, 0, sizeof(resumen));
  memset(buffer, 0, sizeof(buffer));

  /*Recepcion de todas las entradas perdidas*/
  for (i = 0; i < nroMensajes; i++) {	
    /* Recepción de datos */
    if ((numbytes=recv(regCliente->sockCliente,buffer,BUFFER_LEN,0))==-1)	{
      perror("recv");
      pthread_exit(NULL); 
    }	

    /* Se visualiza lo recibido. */
    printf("\npaquete proveniente de : %s\n",inet_ntoa((regCliente->idCliente).sin_addr));
    printf("longitud del paquete en bytes: %d\n",numbytes);
    printf("el paquete contiene: %s\n", buffer);
		

    buffer[numbytes]='\0';
    if ((resumen[i]=(char*)malloc(sizeof(char)*(numbytes+1)))==NULL) {    
      fprintf(stderr, "Datos sin retornar: %s\n", strerror(errno));
      pthread_exit(NULL); 
    }

    memset(resumen[i], 0, sizeof(resumen[i]));
    strncpy(resumen[i],buffer,numbytes);		
    memset(buffer, 0, sizeof(buffer));
  }	

  free(buffer);
  *recuperar=FALSE;

  /*Arreglo para mensajes con alarmas. Se pasa al programa principal. */
  if ((salida=(char**)malloc(sizeof(char*)*nroMensajes))==NULL) {    
    fprintf(stderr, "Datos sin retornar: %s\n", strerror(errno));
    pthread_exit(NULL); 
  }

  /*Procesamiento de los mensajes recibidos a las bitacoras*/
  for (i = 0; i < nroMensajes; i++) {	
    if ((salida[i]=(char*)malloc(sizeof(char)*(numbytes+1)))==NULL) {    
      fprintf(stderr, "Datos sin retornar: %s\n", strerror(errno));
      pthread_exit(NULL); 
    }
    memset(salida[i], 0, sizeof(salida[i]));
    procesarMensaje(regCliente->bitacoraA,regCliente->bitacoraG,
		    resumen[i],numbytes,salida[i],recuperar);
  }
  destruirResumen(resumen, nroMensajes);

  informacion->recuperacion=TRUE;
  informacion->mensaje=NULL;
  informacion->mensajeArreglo=salida;
  informacion->nroMensajes=nroMensajes;
  informacion->idCliente=regCliente->idCliente;

  printf("FIN MODO DE RECUPERACION\n");
  return informacion;
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

int destruirResumen(char** resu, int tam){
  
  if (resu == NULL) 
    return FALSE;
   
  int i=0;
  for (i = 0; i < tam; ++i) {
    free(resu[i]);
  }
  free(resu);
  resu=NULL;
  return TRUE;
}

int destruirInformes(Msg *caj, int tam){
  
  if (caj == NULL) 
    return FALSE;    
	
  if (caj->mensajeArreglo!=NULL){		
    int i=0;
    for (i = 0; i < tam; ++i) {
      free(caj->mensajeArreglo[i]);
    }
    free(caj->mensajeArreglo);
  }

  if (caj->mensaje!=NULL) {
    free(caj->mensaje);
  }
  free(caj);
  return TRUE;
}

int compararMensajes(char *palabra, char *p) {
    
  int i=0;
  if (palabra == NULL)
    return 2;
    
  i=strncasecmp(palabra,p,strlen(palabra));
    
  /* printf("%s ",caj1->palabra); */
  /* printf("%s ",p); */
  /* printf("%d\n",i); */
    
  if (i != 0)
    return FALSE;
  else 
    return TRUE;
}


int sendmail(const char *to, const char *from, const char *subject, const char *message)
{
    int retval = -1;
    FILE *mailpipe = popen("/usr/lib/sendmail -t", "w");
    if (mailpipe != NULL) {
        fprintf(mailpipe, "To: %s\n", to);
        fprintf(mailpipe, "From: %s\n", from);
        fprintf(mailpipe, "Subject: %s\n\n", subject);
        fwrite(message, 1, strlen(message), mailpipe);
        fwrite(".\n", 1, 2, mailpipe);
        pclose(mailpipe);
        retval = 0;
     }
     else {
         perror("Failed to invoke sendmail");
     }
     return retval;
}

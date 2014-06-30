/**
  * Implementación de las funciones y constantes del programa svr_s
  *
  * @file    Operacionesvr_s.c
  * @author  Luiscarlo Rivera 09-11020, Daniel Leones 09-10977
  *
  */
#include "Operacionesvr_s.h"

/*Semaforo para garantizar integridad de las bitacoras*/
pthread_mutex_t candado;

/**
 * Abre un socket TCP para recibir información
 * @param  nroPuerto  Número del puerto del servidor
 * @return Descriptor del socket si tuvo exito, -1 si falló
 */
int Abrir_Socket(int nroPuerto) {
  int sockfd; /* descriptor para el socket */
  struct sockaddr_in my_addr; /* direccion IP y numero de puerto local */

  /* se crea el socket */
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("Socket no creado");
    return -1;
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
    return -1;
  }

  /*Se ordena la escucha para atender las solicitudes de conexión */
  if (listen(sockfd,5)==-1) {
    perror("listen");
    return -1;
  }
  return sockfd;
}

/**
 * Atiende, mediante hilos, las peticiones de los clientes.
 * @param  Estructura Args Id. Cliente.
 * @return Estructura Msg Enviar alarmas encontradas, NULL si falló
 */
void* Atender_Clientes(void* dat)
{	
  Args *registroCliente = (Args *) dat; 
  Msg *informacion=NULL; 
  int numbytes=0;
  char *buffer=NULL;
  char* alarma=NULL;
  int recuperacion=FALSE, err=0;

  /* Buffer de recepción */
  buffer=Pedir_Memoria(BUFFER_LEN);
  if (buffer==NULL){
    fprintf(stderr, "No hay memoria disponible: %s\n", strerror(errno));
    pthread_exit(NULL); 
  }

  /* Recepción de datos */
  if ((numbytes=recv(registroCliente->sockCliente,buffer,BUFFER_LEN,0))==-1)	{
    err=errno;
    manejarClienteCaido(err,registroCliente->idCliente);
    perror("recv");
    pthread_exit(NULL); 
  }

  alarma=Pedir_Memoria(numbytes);
  if (alarma==NULL){
    fprintf(stderr, "No hay memoria disponible: %s\n", strerror(errno));
    pthread_exit(NULL); 
  }

  buffer[numbytes]='\0';
  /* Se visualiza lo recibido y se da la orden de escritura en la bitacora */
  printf("\npaquete proveniente de : %s\n",inet_ntoa((registroCliente->idCliente).sin_addr));
  printf("longitud del paquete en bytes: %d\n",numbytes);	
  printf("el paquete contiene: %s\n", buffer);

  if (numbytes>0){
    if (!(procesarMensaje(registroCliente->bitacoraA, registroCliente->bitacoraG,
    		buffer,numbytes,alarma,&recuperacion))) {
      pthread_exit(NULL); 
    }
  } else {
    pthread_exit(NULL);
  }

  if (recuperacion){
    informacion=modoRecuperacion(&recuperacion,registroCliente);
  } else {
    /*Modo normal*/
    /*Crear estructura de retorno al programa principal */
    informacion=contruirDatosSalientes(numbytes);
    if (informacion==NULL){
        fprintf(stderr, "Datos sin retornar: %s\n", strerror(errno));
        pthread_exit(NULL); 
    }
    strncpy(informacion->mensaje,alarma,strlen(alarma)+1);
    informacion->idCliente=registroCliente->idCliente;

    free(buffer);
    free(alarma);
  }
  pthread_exit(informacion); 
}

/**
 * Clasificacion de los patrones de errores y escritura en la bitacora de alarmas
 * @param  Estructura Args Id. Cliente.
 * @param  bitacoraA Descrpitor de bitacora de alarmas.
 * @param  bitacoraG Descrpitor de bitacora general.
 * @param  buffer Espacio reservado para el mensaje a procesar
 * @param  numbytes Numero de caracteres leidos.
 * @return salida Mensaje de alarma. Si hubiere. En caso contrario, NULL.
 * @return recuperar Enviar alarmas encontradas. En caso contrario, FALSE.
 * @return TRUE exito. FALSE en caso contrario.
 */
int procesarMensaje(FILE *bitacoraA, FILE* bitacoraG, 
		    char* buffer, int numbytes, char* salida, int *recuperar){

  char *buffer2=NULL; 
  char *subcadena=NULL; 
  char *cadena=NULL;
  char* entradaBita=NULL;	
  int k=0,i=0, cent=TRUE;

  if ((buffer==NULL) || (numbytes>0)) {
    //printf("resumen: %s\n", buffer);
    entradaBita=Pedir_Memoria(numbytes+1);
    if (entradaBita==NULL){
      fprintf(stderr, "No hay memoria disponible: %s\n", strerror(errno));
      salida=NULL;
      return FALSE;
    }
    /* Disecciona los campos del formato hasta encontrar la descripcion del mensaje. */
    for (k = 0, cadena=buffer; k<(NROCAMPOS-1) ; k++, cadena=NULL) {
      if (NULL!=(subcadena=strtok_r(cadena, " ", &buffer2))) {
        	strncat(entradaBita,subcadena, strlen(subcadena));
        	strncat(entradaBita," ",2);		
        	//printf("subcadena: %s\n", subcadena);
      }
    }
    /*	
     *	Clasificacion de los patrones de errores y escritura en la
     *	bitacora de alarmas. Si no encuentra un error pasa
     *	escribir en la bitacora general. Se pasa la descripcion del
     *	mensaje a Atender_hilos.
     */
    buffer2[strlen(buffer2)]='\0';
    for (i = 0; i < MROMENSAJERRORES ; i++)	{
      if (compararMensajes(mensajesErrores[i],buffer2)) {				
      	strncat(entradaBita,buffer2,strlen(buffer2)+1);
      	strncpy(salida,buffer2,strlen(buffer2)+1);
        salida[strlen(salida)]='\0';
      	pthread_mutex_lock(&candado);	
      	fprintf(bitacoraA, "%s\n", entradaBita);
      	fflush(bitacoraA);
      	//sendmail(TO,FROM,SUBJECT, entradaBita);
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
      //printf("entrada a bitacora(No Error): (%s)\n", entradaBita);
      strncat(entradaBita,buffer2,strlen(buffer2)); 
      pthread_mutex_lock(&candado);	
      fprintf(bitacoraG, "%s\n", entradaBita);
      fflush(bitacoraG);
      pthread_mutex_unlock(&candado);
    }

    //printf("entrada Final a bitacora: (%s) %d -- %d\n", entradaBita, (int)strlen(entradaBita), numbytes);	
    free(entradaBita);
    subcadena=NULL;
    cadena=NULL;		
  }
  return TRUE;
}

/* Para hacer la recuperacion:
   -Se envia el mensaje Modo Recuperacion
   -Se envia la cantidad de mensajes perdidos 

   Mantener el socket del cliente abierto*/

/**
 * Ejecutar el modo recuperacion del servidor. 
 * @param  Estructura Args Id. Cliente.
 * @return recuperar (Des)activacion del modo.
 * @return Estructura Msg Datos para programa principal. NULL en caso contrario.
 */
Msg* modoRecuperacion(int *recuperar, Args *regCliente){
	
  int numbytes=0;
  char* buffer=NULL; 
  char** resumen=NULL;
  char** salida=NULL;	
  Msg *informacion=NULL; 
  int i=0, nroMensajes=0, err=0;
  //printf("MODO DE RECUPERACION\n");

  /*Buffer de recepción */
  buffer=Pedir_Memoria(BUFFER_LEN);
  if (buffer==NULL){
    fprintf(stderr, "No hay memoria disponible: %s\n", strerror(errno));
    pthread_exit(NULL); 
  }
  /*Recepcion del numero de entradas perdidas*/
  if ((numbytes=recv(regCliente->sockCliente,buffer,BUFFER_LEN,0))==-1)	{
    err=errno;
    manejarClienteCaido(err,regCliente->idCliente);
    perror("recv");
    pthread_exit(NULL); 
  }

  buffer[numbytes]='\0';
  //printf("Nro mensajes: %s\n", buffer);
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
      err=errno;
      manejarClienteCaido(err,regCliente->idCliente);
      perror("recv");
      pthread_exit(NULL); 
    }

    buffer[numbytes]='\0';
    resumen[i]=Pedir_Memoria(numbytes+1);
    if (resumen[i]==NULL){
      fprintf(stderr, "Datos sin retornar: %s\n", strerror(errno));
      pthread_exit(NULL);
    }
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
    salida[i]=Pedir_Memoria(numbytes+1);
    if (salida[i]==NULL){
      fprintf(stderr, "Datos sin retornar: %s\n", strerror(errno));
      pthread_exit(NULL); 
    }
    procesarMensaje(regCliente->bitacoraA,regCliente->bitacoraG,
		    resumen[i],numbytes,salida[i],recuperar);
  }
  destruirResumen(resumen, nroMensajes);

  /*Crear estructura de retorno al programa principal*/ 
  informacion=contruirDatosSalientesR(nroMensajes);
  if (informacion==NULL){
    fprintf(stderr, "Datos sin retornar: %s\n", strerror(errno));
    pthread_exit(NULL);
  }
  informacion->mensajeArreglo=salida;
  informacion->idCliente=regCliente->idCliente;

  //printf("FIN MODO DE RECUPERACION\n");
  return informacion;
}

/**
 * Ejecutar el modo recuperacion del servidor. 
 * @param  struct sockaddr_in idCliente Id. Cliente.
 * @param  int Nro de error.
 * @return vacio
 */
void manejarClienteCaido(int err, struct sockaddr_in idCliente){

  char *mensaje= "Cliente Down";
  char output[128];
  time_t tiempo = time(0);
  struct tm *tlocal = localtime(&tiempo);  
  strftime(output,128,"%H:%M %d/%m/%y",tlocal);
  sprintf(mensaje,"%s ATM %s %s", inet_ntoa(idCliente.sin_addr),output,mensaje);
  switch (err) {
    case EHOSTDOWN:       
      sendmail(TO,FROM,SUBJECT,mensaje);
      break;
    case EHOSTUNREACH: 
      sendmail(TO,FROM,SUBJECT,mensaje);
      break;
    }
}

/**
 * Contruye las estructuras Msg con mensaje unitario
 * @param  int tamaño del espacio a reservar.
 * @return Estructura Msg Datos para programa principal. NULL en caso contrario.
 */
Msg* contruirDatosSalientes(int longiMensaje){
  Msg* informacion=NULL;
     /*Crear estructura de retorno al programa principal */
  if (longiMensaje>0) {
    if ((informacion=(Msg*)malloc(sizeof(Msg)))==NULL) {  
      fprintf(stderr, "Datos sin retornar: %s\n", strerror(errno));
      return NULL;
    }
    informacion->mensajeArreglo=NULL;
    informacion->nroMensajes=1;
    informacion->recuperacion=FALSE;

    if ((informacion->mensaje=(char*)malloc(sizeof(char)*(longiMensaje+1)))==NULL) {    
      fprintf(stderr, "Datos sin retornar: %s\n", strerror(errno));
      return NULL;
    }
    memset(informacion->mensaje, 0, sizeof(informacion->mensaje));
    return informacion;
  }
  return NULL;
}
/**
 * Contruye las estructuras Msg con mensajes en arreglo
 * @param  int tamaño del espacio a reservar para arreglo.
 * @return Estructura Msg Datos para programa principal. NULL en caso contrario.
 */
Msg* contruirDatosSalientesR(int tamArreglo){
  Msg* informacion=NULL;
     /*Crear estructura de retorno al programa principal */
  if (tamArreglo>1) {
    if ((informacion=(Msg*)malloc(sizeof(Msg)))==NULL) {  
      fprintf(stderr, "Datos sin retornar: %s\n", strerror(errno));
      return NULL;
    }
    informacion->mensaje=NULL;
    informacion->nroMensajes=tamArreglo;
    informacion->recuperacion=TRUE;
    return informacion;
  }
  return NULL;
}

/**
 * Asignacion de datos entrantes para hilos
 * @param  struct sockaddr_in Id.Cliente
 * @param  bitaA Descrpitor de bitacora de alarmas.
 * @param  bitaG Descrpitor de bitacora general.
 * @param  sockCli Socket del cliente.
 * @return Estructura Args Id. Cliente.
 * @return TRUE exito. FALSE en caso contrario.
 */
int asignarDatosEntrantes(Args *caj, int sockCli, struct sockaddr_in idCli, FILE *bitaG, FILE *bitaA){
  
  if (caj == NULL) 
    return FALSE;

  caj->sockCliente=sockCli;
  caj->idCliente=idCli;
  caj->bitacoraA=bitaA;
  caj->bitacoraG=bitaG;
  return TRUE;
}

char* Pedir_Memoria(int tam){
  char *str = NULL;
  if ((str=(char*)malloc(sizeof(char)*tam))==NULL) {    
    fprintf(stderr, "No hay memoria disponible: %s\n", strerror(errno));
    return NULL;
  }
  memset(str,0,sizeof(str));
  return str;
}

/**
 * Libera memoria de datos entrantes para hilos
 * @param  Estructura Args Id.Cliente
 * @return TRUE exito. FALSE en caso contrario.
 */

int destruirDatosEntrantes(Args *caj){
  
  if (caj == NULL) 
    return FALSE;
   
  close(caj->sockCliente);
  caj->bitacoraG=NULL;
  caj->bitacoraA=NULL;
  return TRUE;
}
/**
 * Libera memoria de arreglo de cadenas.
 * @param  resu Arreglo de cadenas.
 * @param  tam tamano del arreglo
 * @return TRUE exito. FALSE en caso contrario.
 */

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

/**
 * Libera memoria de arreglo de datos salientes.
 * @param  caj arreglo de datos salientes.
 * @param  tam tamano del arreglo
 * @return TRUE exito. FALSE en caso contrario.
 */
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

/**
 * Programa para enviar correos con alarmas detectadas.
 * @param  to Direccion de correo
 * @param  from Nombre del servidor.
 * @return TRUE exito. FALSE en caso contrario.
 */
 int sendmail(const char *to, const char *from, const char *subject, const char *message)
{
  int retval = 1;
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

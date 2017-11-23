
/**
 * Para aumentar el numero maximo de corredores con el programa en ejecucion se ha modificado
 * la señal SIGVTALRM.
 *
 * Para aumentar el numero maximo de boxes con el programa en ejecucion se ha modificado la
 * la señal SIGALRM.
 */

#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>


/**
 * estadisticas contiene los valores de las mejores posicion, vuelta rapida y metor tiempo de box.
 */
struct estadisticas {

  	char corredor1[13];
  	int tiempoCorredor1;

  	char corredor2[13];
 	int tiempoCorredor2;

  	char corredor3[13];
  	int tiempoCorredor3;

  	char corredorVueltaRapida[13];
  	int tiempoVueltaRapida;

  	char boxMasRapido[13];
  	int tiempoBox;
  	char corredorBox[13];


} estadisticas;

/**
 * corredor es la estructura que tiene un corredor.
 * id contiene el id que representa a cada corredor.
 * numero es el numero que ha sido asignado a cada corredor.
 * siguiente contiene la direccion de memoria del corredor que entrara despues.
 */
struct corredor {

  	char id[13];
  	int numero;
  	int enBoxes;
  	int tiempoPorVuelta;
  	struct corredor* siguiente;

};

/**
 * listaCorredores lista en la que se alojan los corredores.
 * cabeza contiene la direccion de memoria del primer corredor en entrar a la pista.
 * cola contiene la direccion de memoria del ultimo corredor que ha entrado en la pista.
 */
struct listaCorredores {

  	struct corredor* cabeza;
  	struct corredor* cola;

} listaCorredores;

/**
 * box es la estructura que tiene un box.
 * id contiene el id que representa a cada box.
 * numero es el numero que ha sido asignado a cada box.
 * corredoresAtendidos lleva la cuenta de los corredores atendidos por el box.
 * siguiente contiene la direccion de memoria del siguiente box.
 */
struct box {

  	char id[13];
  	int numero;
  	int corredoresAtendidos;
  	struct box* siguiente;

};

/**
 * esperaBoxes es la estructura que tiene un corredor que quiere entrar a boxes.
 * corredorEnEspera contiene la direccion de memoria de un corredor.
 * siguiente contiene la direccion de memoria del corredor que entrara despues.
 */
struct esperaBoxes {

  	struct corredor* corredorEnEspera;
  	struct esperaBoxes* siguiente;

};


/**
 * listaEsperaBoxes lista de corredores que quieren parar en boxes.
 * cabeza contiene la direccion de memoria del primer corredor en parar.
 * cola contiene la direccion de memoria del ultimo corredor que quiere parar.
 */
struct listaEsperaBoxes {

  	struct esperaBoxes* cabeza;
  	struct esperaBoxes* cola;

} listaEsperaBoxes;

/**
 * Numero de vueltas que se tienen que dar a la pista
 */
#define NUM_VUELTAS 5

/**
 * maxCorredores indica la capacidad maxima que tiene la pista para alojar los corredores a la vez.
 */
int maxCorredores;

/**
 * maxBoxes indica el numero maximo de boxes que contiene la pista.
 */
int maxBoxes;

/**
 * numeroDeCorredor indica el numero que le corresponde a cada corredor
 */
int numeroDeCorredor;

/**
 * numeroDeBox indica el numero que le corresponde a cada box
 */
int numeroDeBox;

/**
 * Indica la cantidad de corredores que estan dentro de la pista
 */
int cantidadDeCorredoresActivos;

/**
 * corredorSancionado contiene el numero del corredor que ha sancionado el juez.
 */
int corredorSancionado;

/*Variable para que se comiencen a probar si ha sido sancionado.*/

int corredorCompruebaEntrada;

/*El juez está listo para sancionar.*/
int sancionJuez;

/**
 * Indica el numero de boxes cerrados
 */
int numeroDeBoxesCerrados;

/**
 * Toma valores 0/1 si no se puede/se puede cerrar un box segun numeroDeBoxesCerrados que haya.
 */
int seCierra;

/**
 * corredorPreparadoPasaSancion toma valor 0 si no esta preparado y 1 si el corredor puede afrontar la sacion
 */
int corredorPreparadoParaSancion;

/*Se crea la condicion del hilo.*/
pthread_cond_t condicion;

/**
 * condicionSancion es la condicion que avisa al corredor de que ha acabado la sancion.
 */
pthread_cond_t condicionSancion;

/**
 * Mutex que controla la modificacion de la mejor parada en box.
 */
pthread_mutex_t mutexMejorTiempoBox;

/**
 * mutexListaCorredores controla las modificaciones de la lista
 */
pthread_mutex_t mutexListaCorredores;

/**
 * Mutex que controla al juez
 */
pthread_mutex_t mutexJuez;

/**
 * mutexSancion controla la sancion del juez.
 */
pthread_mutex_t mutexSancion;

/**
 * Mutex que controla la comprobacion de la mejor vuelta.
 */
pthread_mutex_t mutexMejorVuelta;

/**
 * Mutex que controla la actualizacioon del mejor tiempo.
 */
pthread_mutex_t mutexMejorTiempo;

/**
 * mutexLog controla las entradas en el log.
 */
pthread_mutex_t mutexLog;

/**
 * mutexListaEsperaBoxes controla las modificaciones de la lista
 */
pthread_mutex_t mutexListaEsperaBoxes;

/**
 * mutexListaCorredoresENEspera controla las modificaciones de la lista
 */
pthread_mutex_t mutexListaCorredoresEnEspera;

/**
 * mutexListaBoxes controla las modificaciones de la variable global numeroDeBoxesCerrados y seCierra
 */
pthread_mutex_t mutexBoxesCerrados;

/**
 * puntero que apunta al fichero registroTiempos.log
 */
FILE *logFile;

void init ();
void imprimirEstadisticas(char emisor[23]);
void mostrarEstdisticas ();
void aumentarCorredores();
void aumentarBoxes ();
void aniadirCorredor (struct corredor* nuevoCorredor);
void eliminarCorredor (struct corredor* corredorAEliminar);
void nuevoCorredor();
void crearBox();
void* accionBox(void* );
void sePuedeCerrarBox();
void abrirBox();
void cerrarBox();
void aniadirListaEsperaBoxes (struct corredor* nuevoCorredor);
struct corredor* atenderCorredor();
void* pista(void* );
void crearJuez();
void* sancionar (void*);
int compruebaCorredorSancion();
void writeLogMessage(char *id, char *msg);
void finPrograma();


int main(int argc, char** argv){

	if (argc == 3) {

  		maxCorredores = atoi(argv[1]);
    	maxBoxes = atoi(argv[2]);

  	}

  	else if (argc == 1) {

  		maxCorredores = 5;
  		maxBoxes = 2;

  	}

  	else {

  		printf("Error. Debe ingresar los argumenos numero_maximo_de_corredores y numero_de_boxes\n");
    	exit(1);

  	}

  	if (signal(SIGUSR1, nuevoCorredor) == SIG_ERR) {

    	printf("Error en la llamada SIGUSR1: %s\n", strerror(errno));

  	}

  	if (signal(SIGUSR2, mostrarEstdisticas) == SIG_ERR) {

    	printf("Error en la llamada SIGUSR2: %s\n", strerror(errno));

  	}

  	if (signal(SIGVTALRM, aumentarCorredores) == SIG_ERR) {

  		printf("Error en la llamada SIGVTALRM: %s\n", strerror(errno));

  	}

  	if (signal(SIGALRM, aumentarBoxes) == SIG_ERR) {

    	printf("Error en la llamada SIGALRM: %s\n", strerror(errno));

  	}

  	if (signal(SIGTERM, finPrograma) == SIG_ERR) {

    	printf("Error en la llamada SIGTERM: %s\n", strerror(errno));

  	}

  	srand(time(NULL));
  	init();
  	crearJuez();

  	while(1){

    	sleep(2);

  	}

  	pthread_exit(NULL);

  	return 0;

}

/**
 * Inicializa todas las variables globales.
 */
void init () {

	remove("registroTiempos.log");

	corredorPreparadoParaSancion = 0;
  	corredorSancionado = 0;
  	numeroDeCorredor = 1;
  	numeroDeBox = 1;
  	sancionJuez = 0;
  	cantidadDeCorredoresActivos = 0;
  	estadisticas.tiempoVueltaRapida = 1000;
  	estadisticas.tiempoCorredor1 = 1000;
  	estadisticas.tiempoCorredor2 = 1000;
  	estadisticas.tiempoCorredor3 = 1000;
  	estadisticas.tiempoBox = 1000;
  	numeroDeBoxesCerrados = 0;
  	seCierra = 1;
  	listaCorredores.cabeza = NULL;
  	listaCorredores.cola = NULL;
  	listaEsperaBoxes.cabeza = NULL;
  	listaEsperaBoxes.cola = NULL;

  	pthread_cond_init(&condicion, NULL);
	pthread_cond_init(&condicionSancion, NULL);


  	pthread_mutex_init(&mutexMejorVuelta, NULL);
  	pthread_mutex_init(&mutexListaCorredores, NULL);
  	pthread_mutex_init(&mutexJuez, NULL);
  	pthread_mutex_init(&mutexSancion, NULL);
  	pthread_mutex_init(&mutexMejorTiempo, NULL);
  	pthread_mutex_init(&mutexLog, NULL);
 	pthread_mutex_init(&mutexListaEsperaBoxes, NULL);
  	pthread_mutex_init(&mutexListaCorredoresEnEspera,NULL);
  	pthread_mutex_init(&mutexBoxesCerrados,NULL);

  	/**
  	 *Creamos los boxes especificados
  	 */
  	int i;
  	for(i=0;i<maxBoxes;i++){
    	crearBox();
  	}

}

void imprimirEstadisticas(char emisor[23]) {

	char mensaje[100], tiempo[2];

	if (estadisticas.tiempoCorredor3 < 1000) {

		pthread_mutex_lock(&mutexMejorTiempo);

		strcpy(mensaje, "En primera posicion ");
		strcat(mensaje, estadisticas.corredor1);
		writeLogMessage(emisor, mensaje);

		strcpy(mensaje, "En segunda posicion ");
		strcat(mensaje, estadisticas.corredor2);
		writeLogMessage(emisor, mensaje);

		strcpy(mensaje, "En tercera posicion ");
		strcat(mensaje, estadisticas.corredor3);
		writeLogMessage(emisor, mensaje);

		printf("Primera posicion: %s.\n", estadisticas.corredor1);
		printf("Segunda posicion: %s.\n", estadisticas.corredor2);
		printf("Tercera posicion: %s.\n", estadisticas.corredor3);

		pthread_mutex_unlock(&mutexMejorTiempo);

	}

	pthread_mutex_lock(&mutexMejorVuelta);

	strcpy(mensaje, "Con la mejor vuelta ");
	strcat(mensaje, estadisticas.corredorVueltaRapida);
	strcat(mensaje, " con un tiempo de ");
	sprintf(tiempo, "%d", estadisticas.tiempoVueltaRapida);
	strcat(mensaje, tiempo);
	strcat(mensaje, " segundos");
	writeLogMessage(emisor, mensaje);

	printf("Vuelta rapida: %s con un tiempo de %d segundos.\n", 
		estadisticas.corredorVueltaRapida, estadisticas.tiempoVueltaRapida);

	pthread_mutex_unlock(&mutexMejorVuelta);

	pthread_mutex_lock(&mutexMejorTiempoBox);

	strcpy(mensaje, "Con la parada en box mas rapida ");
	strcat(mensaje, estadisticas.boxMasRapido);
	strcat(mensaje, " con ");
	strcat(mensaje, estadisticas.corredorBox);
	strcat(mensaje, " con un tiempo de ");
	sprintf(tiempo, "%d", estadisticas.tiempoBox);
	strcat(mensaje, tiempo);
	strcat(mensaje, " segundos");
	writeLogMessage(emisor, mensaje);

	printf("Parada en box mas rapida: %s con un tiempo de %d con %s.\n",
		estadisticas.boxMasRapido, estadisticas.tiempoBox, estadisticas.corredorBox);

	pthread_mutex_unlock(&mutexMejorTiempoBox);

}

void mostrarEstdisticas () {

	if (signal(SIGUSR2, SIG_IGN) == SIG_ERR) {

    	printf("Error: %s\n", strerror(errno));

  	}

	imprimirEstadisticas("SIGUSR2");

	if (signal(SIGUSR2, mostrarEstdisticas) == SIG_ERR) {

    	printf("Error: %s\n", strerror(errno));

  	}

}

void aumentarCorredores () {

  	if (signal(SIGVTALRM, SIG_IGN) == SIG_ERR) {

    	printf("Error: %s\n", strerror(errno));

  	}

  	maxCorredores++;

  	char mensaje[50], numCorredores[5];
  	strcpy(mensaje, "Se ha modificado el nº de corredores a ");
  	sprintf(numCorredores, "%d", maxCorredores);
  	strcat(mensaje, numCorredores);

 	writeLogMessage("Mensaje", mensaje);

  	if (signal(SIGVTALRM, aumentarCorredores) == SIG_ERR) {

    	printf("Error: %s\n", strerror(errno));

  	}

}

void aumentarBoxes () {

  	if (signal(SIGALRM, SIG_IGN) == SIG_ERR) {

    	printf("Error: %s\n", strerror(errno));

  	}

  	maxBoxes++;

  	char mensaje[50], numBoxes[5];
   	strcpy(mensaje, "Se ha modificado el nº de boxes a ");
   	sprintf(numBoxes, "%d", maxBoxes);
  	strcat(mensaje, numBoxes);

  	writeLogMessage("Mensaje", mensaje);

  	crearBox();

  	if (signal(SIGALRM, aumentarBoxes) == SIG_ERR) {

    	printf("Error: %s\n", strerror(errno));

  	}

}

/**
 * Añade en la cola un nuevo corredor pasador por parametro
 */
void aniadirCorredor (struct corredor* nuevoCorredor) {

  	pthread_mutex_lock(&mutexListaCorredores);

  	if (listaCorredores.cabeza == NULL) {

    	listaCorredores.cabeza = nuevoCorredor;
    	listaCorredores.cola = nuevoCorredor;
    	nuevoCorredor->siguiente = NULL;

  	}

  	else {

    	listaCorredores.cola->siguiente = nuevoCorredor;
    	listaCorredores.cola = nuevoCorredor;
    	nuevoCorredor->siguiente = NULL;

  	}

 	pthread_mutex_unlock(&mutexListaCorredores);

}

/**
 * eliminarCorredor elimina el corredor pasador por parametro de la lista de corredores
 */
void eliminarCorredor (struct corredor* corredorAEliminar) {

  	pthread_mutex_lock(&mutexListaCorredores);

  	struct corredor* aux;

  	if (listaCorredores.cabeza == corredorAEliminar) {

    	listaCorredores.cabeza = listaCorredores.cabeza->siguiente;

  	}

  	else if (listaCorredores.cola == corredorAEliminar) {

    	aux = listaCorredores.cabeza;

    	while (aux->siguiente != corredorAEliminar) {

      	aux = aux->siguiente;

    	}

    	listaCorredores.cola = aux;
    	aux->siguiente = NULL;

  	}

  	else {

    	aux = listaCorredores.cabeza;

    	while ((aux->siguiente != corredorAEliminar) && (aux->siguiente != NULL)) {

      	aux = aux->siguiente;

    	}

    	aux->siguiente = corredorAEliminar->siguiente;

  	}

  	free(corredorAEliminar);

  	pthread_mutex_unlock(&mutexListaCorredores);

}

/**
 * Crea un nuevo corredor asignandole el numero que le corresponda.
 */
void nuevoCorredor(){

  	if (signal(SIGUSR1, SIG_IGN) == SIG_ERR) {

    	printf("Error: %s\n", strerror(errno));

  	}

    if (cantidadDeCorredoresActivos < maxCorredores) {

    	pthread_t hcorredor;
      	struct corredor* nCorredor;
      	char id[13];
      	char c_numero[3];

      	nCorredor = (struct corredor*)malloc(sizeof(struct corredor));

      	sprintf(c_numero, "%d", numeroDeCorredor);
      	strcpy(id, "corredor_");
      	strcat(id, c_numero);
      	strcpy(nCorredor->id, id);

      	nCorredor->numero = numeroDeCorredor;

      	if(pthread_create (&hcorredor, NULL, pista, nCorredor) != 0){

        	printf("Error al crear el hilo. %s\n", strerror(errno));

      	}

      	else {

        	numeroDeCorredor++;
        	cantidadDeCorredoresActivos++;
        	aniadirCorredor(nCorredor);
        	printf("%s entra en el circuito.\n", nCorredor->id);
        	writeLogMessage(nCorredor->id, "Entra en el circuito");

      	}

    }

  	if (signal(SIGUSR1, nuevoCorredor) == SIG_ERR) {

   		printf("Error: %s", strerror(errno));

  	}

}

/**
 * Crea un box
 */
void crearBox () {

 	pthread_t hBox;
    struct box* nBox;
    char id[13];
    char box_numero[3];

    nBox = (struct box*)malloc(sizeof(struct box));

    sprintf(box_numero, "%d", numeroDeBox);
    strcpy(id, "box_");
    strcat(id, box_numero);
    strcpy(nBox->id, id);

    nBox->numero = numeroDeBox;

    if(pthread_create (&hBox, NULL, accionBox, nBox) != 0){

    	printf("Error al crear el hilo. %s\n", strerror(errno));

    }

    else {
    	
    	writeLogMessage(nBox->id, " He sido creado");
      	numeroDeBox++;

    }
}

/**
 * accionBox define el comportamiento de los hilos de los boxes.
 */
void *accionBox(void* parametro){

  	struct box* nBox = (struct box*)parametro;
  	struct corredor* nCorredor;

  	int corredoresAtendidos = 0;

  	int tiempoEnBoxes = 0;

  	while(1) {

    	if (listaEsperaBoxes.cabeza == NULL) {

      		sleep(1);

    	}

    	else{

        	nCorredor = atenderCorredor();

        	char mensaje[40];
        	strcpy(mensaje, "Entra ");
        	strcat(mensaje, nCorredor->id);
        	writeLogMessage(nBox->id, mensaje);

        	corredoresAtendidos++;
        	tiempoEnBoxes = rand()%3+1;
        	sleep(tiempoEnBoxes);

        	pthread_mutex_lock(&mutexMejorTiempoBox);

        	if (tiempoEnBoxes < estadisticas.tiempoBox) {

        		estadisticas.tiempoBox = tiempoEnBoxes;
        		strcpy(estadisticas.corredorBox, nCorredor->id);
        		strcpy(estadisticas.boxMasRapido, nBox->id);

        		char tiempoBox[2];
        		sprintf(tiempoBox, "%d", tiempoEnBoxes);
        		strcpy(mensaje, "Ha superado el tiempo de la parada en box con ");
        		strcat(mensaje, tiempoBox);
        		strcat(mensaje, " segundos con el corredor ");
        		strcat(mensaje, nCorredor->id);
        		strcat(mensaje, ".");
        		writeLogMessage(nBox->id, mensaje);

        	}

        	pthread_mutex_unlock(&mutexMejorTiempoBox);

        	nCorredor->enBoxes = 0;
        	nCorredor->tiempoPorVuelta = tiempoEnBoxes+ + nCorredor-> tiempoPorVuelta;

        	strcpy(mensaje, nCorredor->id);
        	strcat(mensaje, " sale del box");
        	writeLogMessage(nBox->id, mensaje);

        	sePuedeCerrarBox();
  
        	if((corredoresAtendidos>=3)&&(seCierra==1)){
          
          		writeLogMessage(nBox->id, "He cerrado");

          		cerrarBox();
          		sleep(20);
          		corredoresAtendidos = 0;
          		abrirBox();
          
          		writeLogMessage(nBox->id, "He abierto");

        	}

    	}

  	}

}

/**
 * Comprueba si se puede cerrar el box o no
 * para asegurar que haya al menos uno abierto.
 */
void sePuedeCerrarBox(){
  
  	pthread_mutex_lock(&mutexBoxesCerrados);

  	if(numeroDeBoxesCerrados<maxBoxes-1){

    	seCierra = 1;

  	}

  	else{

    	seCierra = 0;

  	}
  	pthread_mutex_unlock(&mutexBoxesCerrados);
}

/**
 * Decrementa el contador de boxes cerrados.
 */
void abrirBox(){

  	pthread_mutex_lock(&mutexBoxesCerrados);

  	numeroDeBoxesCerrados--;

  	pthread_mutex_unlock(&mutexBoxesCerrados);

}

/**
 * Incrementa el contador de boxes cerrados
 */
void cerrarBox(){

  	pthread_mutex_lock(&mutexBoxesCerrados);

  	numeroDeBoxesCerrados++;

  	pthread_mutex_unlock(&mutexBoxesCerrados);

}


/**
 * aniadirListaEsperaBoxes añade en la cola un nuevo corredor que tiene que parar en boxes
 */
void aniadirListaEsperaBoxes (struct corredor* nuevoCorredor) {

  	pthread_mutex_lock(&mutexListaEsperaBoxes);

  	struct esperaBoxes* nesperaboxes = (struct esperaBoxes*)malloc(sizeof(struct esperaBoxes));
  	nesperaboxes->corredorEnEspera = nuevoCorredor;

  	if (listaEsperaBoxes.cabeza == NULL) {

    	listaEsperaBoxes.cabeza = nesperaboxes;
    	listaEsperaBoxes.cola = nesperaboxes;
    	nesperaboxes->siguiente = NULL;

  	}

  	else {

    	listaEsperaBoxes.cola->siguiente = nesperaboxes;
    	listaEsperaBoxes.cola = nesperaboxes;
    	nesperaboxes->siguiente = NULL;

  	}

  	pthread_mutex_unlock(&mutexListaEsperaBoxes);

}

/**
 * atenderCorredor elimina el primer corredor de la lista de espera
 */
struct corredor* atenderCorredor() {

  	pthread_mutex_lock(&mutexListaCorredoresEnEspera);

  	struct corredor* corredorAtendido = listaEsperaBoxes.cabeza->corredorEnEspera;
  	listaEsperaBoxes.cabeza = listaEsperaBoxes.cabeza->siguiente;

  	pthread_mutex_unlock(&mutexListaCorredoresEnEspera);

  	return corredorAtendido;

}

/**
 * Cada corredor guarda su numero asignado, da 5 vueltas a la pista y sale
 * dejando un hueco libre.
 */
void *pista(void* parametro){

	/**
	 * nCorredor contiene el corredor que le ha sido asignado al hilo
	 */
	struct corredor* nCorredor = (struct corredor*)parametro;

	/**
	 * Contiene el numero de vuetas que lleva el corredor.
	 */
	int numeroDeVueltas = 1;

	/**
	 * tiempoTotal contiene el tiempo que ha tardado el corredor en dar las 5 vueltas.
	 */
	int tiempoTotal = 0;

	/**
	 * entrarEnBoxes si su valor es 1 entra en boxes, si no continua en pista.
	 */
	int entrarEnBoxes = 0;

	/**
	 * tieneProblemasGraves si su valor es 1 indica que tiene problemas graves y avandonara la carrera
	 */
	int tieneProblemasGraves = 0;

	char mensaje[50], tiempoVuelta[50], numVuelta[50];

	while (numeroDeVueltas <= NUM_VUELTAS) {

		/* calculamos el tiempo de vuelta y si se entra a box*/
	    nCorredor->tiempoPorVuelta = rand()%4+2;
	    tiempoTotal = tiempoTotal + nCorredor->tiempoPorVuelta;
	    sleep(nCorredor->tiempoPorVuelta);
	    entrarEnBoxes = rand()%2;

	    
	    if (entrarEnBoxes == 1) {

	    	writeLogMessage(nCorredor->id, "Entro a boxes");

	    	aniadirListaEsperaBoxes(nCorredor);
	      	nCorredor->enBoxes=1;
	      	
	      	while(nCorredor->enBoxes==1) {

	        	sleep(1);

	      	}

	      	tieneProblemasGraves = rand()%10+1;

	      	if(tieneProblemasGraves>7){

	        	writeLogMessage(nCorredor->id, "Abandono la carrera por problemas graves");
	        	printf("%s: Abandono la carrera por problemas graves\n",nCorredor->id);
	        	eliminarCorredor(nCorredor);
	        	cantidadDeCorredoresActivos--;
	        	pthread_exit(NULL);

	      	}

	    }
	    
	    /*comprobamos si el juez va a sanciona*/
		if (compruebaCorredorSancion()==1) {
	      	
	      	while (corredorSancionado==0) {

	        	sleep(1);

	      	}

		}

		/*comprobamos si hemos sido sancionados*/
	    if ( corredorSancionado == nCorredor->numero) {

	    	corredorPreparadoParaSancion = 1;
	    	writeLogMessage(nCorredor->id, "Estoy sancionado");
	      	pthread_cond_wait(&condicionSancion, &mutexSancion);
	      	writeLogMessage(nCorredor->id, "He cumplido la sancion");
	      	nCorredor->tiempoPorVuelta = nCorredor->tiempoPorVuelta+3;
	      	corredorPreparadoParaSancion = 0;
	      	corredorSancionado = 0;

	    }

	    sprintf(numVuelta, "%d", numeroDeVueltas);
	    sprintf(tiempoVuelta, "%d", nCorredor->tiempoPorVuelta);
	    strcpy(mensaje, "Termina la vuelta ");
	    strcat(mensaje, numVuelta);
	    strcat(mensaje, " en ");
	    strcat(mensaje, tiempoVuelta);
	    strcat(mensaje, " segundos.");
	    writeLogMessage(nCorredor->id, mensaje);

	    pthread_mutex_lock(&mutexMejorVuelta);

	    /*calculamos si se ha superado la vuelta rapida*/
	    if (nCorredor->tiempoPorVuelta < estadisticas.tiempoVueltaRapida) {

	    	estadisticas.tiempoVueltaRapida = nCorredor->tiempoPorVuelta;
	    	strcpy(estadisticas.corredorVueltaRapida, nCorredor->id);

	    	sprintf(tiempoVuelta, "%d", nCorredor->tiempoPorVuelta);
	    	strcpy(mensaje, "Ha mejorado la vuelta rapida con ");
	    	strcat(mensaje, tiempoVuelta);
	    	strcat(mensaje, " segundos.");
	    	writeLogMessage(nCorredor->id, mensaje);    	

	    }

	    pthread_mutex_unlock(&mutexMejorVuelta);

	    numeroDeVueltas++;     

	}

    writeLogMessage(nCorredor->id, "Termina la carrera");
    printf("%s: ha acabado la carrera.\n", nCorredor->id);
  	
  	pthread_mutex_lock(&mutexMejorTiempo);

  	/*calculamos si hemos superado los tiempos del 1, 2 o 3*/
  	if (tiempoTotal < estadisticas.tiempoCorredor1) {

    	strcpy(estadisticas.corredor1, nCorredor->id);
    	estadisticas.tiempoCorredor1 = tiempoTotal;

  	}

  	else if (tiempoTotal < estadisticas.tiempoCorredor2) {

  		strcpy(estadisticas.corredor2, nCorredor->id);
    	estadisticas.tiempoCorredor2 = tiempoTotal;

  	}

  	else if (tiempoTotal < estadisticas.tiempoCorredor3) {

  		strcpy(estadisticas.corredor3, nCorredor->id);
    	estadisticas.tiempoCorredor3= tiempoTotal;

  	}

 	pthread_mutex_unlock(&mutexMejorTiempo);

  	eliminarCorredor(nCorredor);
  	cantidadDeCorredoresActivos--;

  	pthread_exit(NULL);

}

/*Comportamiento del juez*/

void crearJuez(){
  	
  	pthread_t hjuez;

  	if (pthread_create (&hjuez, NULL, sancionar, NULL) != 0){

    	printf("Error al crear el hilo juez %s\n ",strerror (errno));

  	}

  	else {

  		writeLogMessage("Juez", " He sido creado");

  	}

}

/*
 *Parte del programa que penaliza a los corredores
 *Dormir en cuanto el juez despierte bloquear el mutex, crear el numero aleatorio
 *esperar a que todos los corredores estén esperando y que cada corredor compruebe si es su número.
 */
void *sancionar(void* parametro){
 	
 	while(1){

 		struct corredor *aux;

 		corredorCompruebaEntrada = 0;
 		
 		writeLogMessage("Juez", " Descanso");

  		sleep(10);
  		
  		
  		sancionJuez=1;
  		/*
  		while (cantidadDeCorredoresActivos < 1) {

  			sleep(1);

  		}*/

  		/*esperamos a que los hilos esten listos pasa ser rancionados*/
  		pthread_cond_wait (&condicion, &mutexJuez);
  		
  		writeLogMessage("Juez", " Estoy preparando la sancion");
  		
  		/*calculamos un numero aleatorio, lo buscamos en la lista de corredores y se le indica que esta sancionado*/
  		int corredorASancionar = rand()%cantidadDeCorredoresActivos+1;
  		aux=listaCorredores.cabeza;

  		int i;
  		for(i=1;i<corredorASancionar;i++){
    		
    		aux=aux->siguiente;

  		}

  		corredorSancionado = aux->numero;

  		/* una vez que el corredor este preparado esperamos los 3 segundos 
  		   e indicamos al corredor de que ha cumplido la sancion*/
  		pthread_mutex_lock(&mutexSancion);
  		char mensaje[50];
  		strcpy(mensaje, aux->id);
  		strcat(mensaje, " ha sido sancionado");
  		writeLogMessage("Juez",mensaje);
  		
  		while (corredorPreparadoParaSancion == 0){

  			sleep(1);

  		}
	
  		
  		sleep(3);
  		strcpy(mensaje, aux->id);
  		strcat(mensaje, " ha cumplido la sancion");
  		writeLogMessage("Juez", mensaje);
  		pthread_cond_signal(&condicionSancion);
  		pthread_mutex_unlock(&mutexSancion);

  	}
}


int compruebaCorredorSancion(){
	
	int haySancion = 0;

	if(sancionJuez == 1){
		
    	pthread_mutex_lock(&mutexJuez);

		if (++corredorCompruebaEntrada >= cantidadDeCorredoresActivos){
		 
			pthread_cond_signal (&condicion);
			haySancion = 1;
			sancionJuez = 0;

		}

		pthread_mutex_unlock (&mutexJuez);

  	}

	return haySancion;

}

/**
* Funcion para escribir en el fichero de logs.
* char *id: cadena que repreenta el id del corredor o del box.
* char *msg: mensaje que indica la accion realizada.
*/
void writeLogMessage(char *id, char *msg) {

   	pthread_mutex_lock(&mutexLog);

   	// Calculamos la hora actual
   	time_t now = time(0);
   	struct tm *tlocal = localtime(&now);
   	char stnow[19];
   	strftime(stnow, 19, "%d/%m/%y  %H:%M:%S", tlocal);

   	// Escribimos en el log
   	logFile = fopen("registroTiempos.log", "a");
   	fprintf(logFile, "[%s] %s: %s\n", stnow, id, msg);
   	fclose(logFile);

   	pthread_mutex_unlock(&mutexLog);

}

/**
* finPrograma captura la señal SIGTERM 
* escribe en el fichero de log el ganador de la carrera y el número de corredores
* que han participado.
*/
void finPrograma() {

  	if (signal(SIGTERM, finPrograma) == SIG_ERR) {
    	
    	printf("Error: %s\n", strerror(errno));
    	exit(-1);

  	}

  	imprimirEstadisticas("SIGTERM");

  	if (numeroDeCorredor > 0) {

    	char corredores[15], mensaje[100];

   		strcpy(mensaje, estadisticas.corredor1);
   		strcat(mensaje, " ha ganador la carrera");
    	writeLogMessage("SIGTERM", mensaje);

    	strcpy(mensaje, "Numero de corredores es ");
    	sprintf(corredores, "%d", (numeroDeCorredor-1));
    	strcat(mensaje, corredores);
    	writeLogMessage("SIGTERM", mensaje);

  	}

  	exit(0);

}

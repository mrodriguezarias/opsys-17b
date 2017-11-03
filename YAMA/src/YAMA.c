#include "YAMA.h"

#include <config.h>
#include <log.h>
#include <process.h>
#include <protocol.h>
#include <socket.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <thread.h>
#include "server.h"
#include "client.h"
#include <semaphore.h>

#define MAXIMO_TAMANIO_DATOS 256 //definiendo el tamanio maximo
int SocketBuscado_GLOBAL = 0;

typedef struct{
	char* FS_IP;
	char* FS_PUERTO;
	char* RETARDO_PLANIFICACION;
	char* ALGORITMO_BALANCEO;
	char* MASTER_PUERTO;
}tinformacion;

typedef struct {
	int id;
	int tamanio;
} t_cabecera;

t_yama yama;
int numeroJob = 1;
mlist_t* listaNodosActivos;
int retardoPlanificacion;
char* algoritmoBalanceo;
void trapper(int signum);
pthread_mutex_t mutexPlanificacion = PTHREAD_MUTEX_INITIALIZER;

int main() {
	thread_signal_capture(SIGUSR1, trapper);
	process_init();
	thread_signal_capture(SIGUSR1, trapper);
	if(connect_to_filesystem() == RESPONSE_ERROR) return EXIT_SUCCESS;;
	listaEstados = mlist_create();
	listen_to_master();
	free(algoritmoBalanceo);
	return EXIT_SUCCESS;
}

void trapper(int signum){
	printf("\nRecibi la señal\n"); //luego quitar probar
	log_inform("Señal atrapada, modificacion del retardo a :%d || modificacion del algoritmo a:%s",retardoPlanificacion,algoritmoBalanceo);
	pthread_mutex_lock(&mutexPlanificacion);
	retardoPlanificacion = atoi(config_get("RETARDO_PLANIFICACION"));
	strcpy(algoritmoBalanceo,config_get("ALGORITMO_BALANCEO"));
	pthread_mutex_unlock(&mutexPlanificacion);
}

void inicializoVariablesGlobalesConfig(){
	retardoPlanificacion = atoi(config_get("RETARDO_PLANIFICACION"));
	algoritmoBalanceo = malloc(sizeof(char)*8);
	strcpy(algoritmoBalanceo,config_get("ALGORITMO_BALANCEO"));
}

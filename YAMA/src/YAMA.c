#include "YAMA.h"

#include <config.h>
#include <log.h>
#include <process.h>
#include <protocol.h>
#include <socket.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "server.h"
#include "client.h"

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
//char* algoritmoBalanceo;
//void trapper(int signum);

int main() {
	//signal(SIGUSR1,trapper);
	process_init();
	//retardoPlanificacion = atoi(config_get("RETARDO_PLANIFICACION"));
	//algoritmoBalanceo = malloc(sizeof(char)*8);
	//strcpy(algoritmoBalanceo,config_get("ALGORITMO_BALANCEO"));
	connect_to_filesystem();
	listaEstados = mlist_create();
	listen_to_master();
	//while(1);
	//free(algoritmoBalanceo);
	return EXIT_SUCCESS;
}

/*void trapper(int signum){
	printf("\nRecibi la señal\n");
	retardoPlanificacion = atoi(config_get("RETARDO_PLANIFICACION"));
	strcpy(algoritmoBalanceo,config_get("ALGORITMO_BALANCEO"));
}*/

#include "YAMA.h"

#include <config.h>
#include <log.h>
#include <process.h>
#include <protocol.h>
#include <socket.h>
#include <stdlib.h>

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

int main() {
	process_init();
	//connect_to_filesystem();
	listaEstados = mlist_create();
	listen_to_master();
	return EXIT_SUCCESS;
}

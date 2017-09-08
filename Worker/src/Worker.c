/*
 ============================================================================
 Name        : Worker.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "socket.h"
#include "serial.h"

#include <commons/config.h>
#include <commons/log.h>

#define MAX_IP_LEN 16   // aaa.bbb.ccc.ddd -> son 15 caracteres, 16 contando un '\0'
#define MAX_PORT_LEN 6  // 65535 -> 5 digitos, 6 contando un '\0'
#define MAX_NOMBRE_NODO 5
#define MAX_RUTA 25

typedef struct{
	char * IP_FILESYSTEM,
		 * PUERTO_FILESYSTEM,
	     * NOMBRE_NODO,
		 * PUERTO_WORKER,
		 * PUERTO_DATANODE,
		 * RUTA_DATABIN;

}tWorker;
tWorker * worker;
t_log * logTrace;
void crearLogger() {
   char *pathLogger = string_new();

   char cwd[1024];

   string_append(&pathLogger, getcwd(cwd, sizeof(cwd)));

   string_append(&pathLogger, "/Worker_LOG.log");

   char *logWorker = strdup("Worker_LOG.log");

   logTrace = log_create(pathLogger, logWorker, false, LOG_LEVEL_INFO);

   free(pathLogger);
   free(logWorker);
}

tWorker *getConfigWorker(char* ruta) {
	printf("Ruta del archivo de configuracion: %s\n", ruta);
	tWorker *worker = malloc(sizeof(tWorker));

	//t_config *workerConfig = config_create("/home/utnso/tp-2017-2c-YATPOS/Worker/src/config_worker");

	t_config *workerConfig = config_create(ruta);

	worker->IP_FILESYSTEM     = malloc(MAX_IP_LEN);
	worker->PUERTO_FILESYSTEM = malloc(MAX_PORT_LEN);
	worker->NOMBRE_NODO       = malloc(MAX_NOMBRE_NODO);
	worker->PUERTO_WORKER     = malloc(MAX_PORT_LEN);
	worker->PUERTO_DATANODE   = malloc(MAX_PORT_LEN);
	worker->RUTA_DATABIN      = malloc(MAX_RUTA);

	strcpy(worker->IP_FILESYSTEM,
			config_get_string_value(workerConfig, "IP_FILESYSTEM"));
	strcpy(worker->PUERTO_FILESYSTEM,
			config_get_string_value(workerConfig, "PUERTO_FILESYSTEM"));
	strcpy(worker->NOMBRE_NODO,
			config_get_string_value(workerConfig, "NOMBRE_NODO"));
	strcpy(worker->PUERTO_WORKER,
			config_get_string_value(workerConfig, "PUERTO_WORKER"));
	strcpy(worker->PUERTO_DATANODE,
			config_get_string_value(workerConfig, "PUERTO_DATANODE"));
	strcpy(worker->RUTA_DATABIN,
			config_get_string_value(workerConfig, "RUTA_DATABIN"));

	config_destroy(workerConfig);
	return worker;
}

void mostrarConfiguracion(tWorker *worker) {
	printf("IP_FILESYSTEM: %s\n", worker->IP_FILESYSTEM);
	printf("NOMBRE_NODO: %s\n", worker->NOMBRE_NODO);
	printf("PUERTO_DATANODE: %s\n", worker->PUERTO_DATANODE);
	printf("PUERTO_FILESYSTEM: %s\n", worker->PUERTO_FILESYSTEM);
	printf("PUERTO_WORKER: %s\n", worker->PUERTO_WORKER);
	printf("RUTA_DATABIN: %s\n", worker->RUTA_DATABIN);
}

void liberarConfiguracionWorker(tWorker*worker) {

	free(worker->IP_FILESYSTEM);
	worker->IP_FILESYSTEM = NULL;
	free(worker->NOMBRE_NODO);
	worker->NOMBRE_NODO = NULL;
	free(worker->PUERTO_DATANODE);
	worker->PUERTO_DATANODE = NULL;
	free(worker->PUERTO_FILESYSTEM);
	worker->PUERTO_FILESYSTEM = NULL;
	free(worker->PUERTO_WORKER);
	worker->PUERTO_WORKER = NULL;
	free(worker->RUTA_DATABIN);
	worker->RUTA_DATABIN = NULL;
}




int main(int argc, char* argv[]) {

	int socket_worker;
	struct sockaddr_in direccion_worker;

	worker = getConfigWorker(argv[1]);
	mostrarConfiguracion(worker);
	crearLogger();

	inicializarSOCKADDR_IN(&direccion_worker, worker->IP_FILESYSTEM,worker->PUERTO_WORKER);
											//La ip, en realidad cambia, pero como es local pongo la de fs

	return EXIT_SUCCESS;
}

#include <stdio.h>
#include <stdlib.h>
#include "socket.h"
#include "serial.h"

#include<commons/log.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include<commons/collections/queue.h>


#include<pthread.h>

#define PORT "3790"
#define MAXSIZE 1024

#define MAX_IP_LEN 16   // aaa.bbb.ccc.ddd -> son 15 caracteres, 16 contando un '\0'
#define MAX_PORT_LEN 6  // 65535 -> 5 digitos, 6 contando un '\0'
#define MAX_RUTA 25


struct data {
	unsigned d;
	short h;
	double f;
	char s[16];
};


typedef struct {
	char * YAMA_IP, *YAMA_PUERTO, *PUERTO_WORKER;
} tMaster;

tMaster*master;

t_log * logTrace;

void crearLogger() {
   char *pathLogger = string_new();

   char cwd[1024];

   string_append(&pathLogger, getcwd(cwd, sizeof(cwd)));

   string_append(&pathLogger, "/Master_LOG.log");

   char *logmaster = strdup("Master_LOG.log");

   logTrace = log_create(pathLogger, logmaster, false, LOG_LEVEL_INFO);

   free(pathLogger);
   free(logmaster);
}
tMaster *getConfigMaster(char* ruta) {
	printf("Ruta del archivo de configuracion: %s\n", ruta);
	tMaster *master = malloc(sizeof(tMaster));

	//t_config *masterConfig = config_create("/home/utnso/tp-2017-2c-YATPOS/master/src/config_master");

	t_config *masterConfig = config_create(ruta);

	master->YAMA_IP     = malloc(MAX_IP_LEN);
	master->YAMA_PUERTO = malloc(MAX_PORT_LEN);
	master->PUERTO_WORKER       = malloc(MAX_PORT_LEN);

	strcpy(master->YAMA_IP,
			config_get_string_value(masterConfig, "YAMA_IP"));
	strcpy(master->YAMA_PUERTO,
			config_get_string_value(masterConfig, "YAMA_PUERTO"));
	strcpy(master->PUERTO_WORKER,
			config_get_string_value(masterConfig, "PUERTO_WORKER"));

	config_destroy(masterConfig);
	return master;
}

void mostrarConfiguracion(tMaster *master) {
	printf("YAMA_IP: %s\n", master->YAMA_IP);
	printf("YAMA_PUERTO: %s\n", master->YAMA_PUERTO);
	printf("PUERTO_WORKER: %s\n", master->PUERTO_WORKER);
}

void liberarConfiguracionmaster(tMaster*master) {

	free(master->YAMA_IP);
	master->YAMA_IP = NULL;
	free(master->YAMA_PUERTO);
	master->YAMA_PUERTO = NULL;
	free(master->PUERTO_WORKER);
	master->PUERTO_WORKER = NULL;

}





int main(int argc,char*argv[]) {
//	char buffer[MAXSIZE];
//	struct data m;
//
//	puts("Hello, I am a serveworkerr.");
//
//	puts("Listening for connections...");
//	socket_t socket = socket_listen(PORT);
//	puts("Connected.");
//
//	puts("Receiving message...");
//	size_t n = socket_receive_bytes(socket, buffer, sizeof(struct data));
//	serial_unpack(buffer, "Ihds", &m.d, &m.h, &m.f, m.s);
//
//	printf("Received %d bytes:\n", n);
//	printf("m.d = %u\n", m.d);
//	printf("m.h = %hd\n", m.h);
//	printf("m.f = %lf\n", m.f);
//	printf("m.s = %s\n", m.s);

	int socket_master;
	struct sockaddr_in direccion_master;

	master = getConfigMaster(argv[1]);
	mostrarConfiguracion(master);
	crearLogger();

	socket_master = crearSocket();
	inicializarSOCKADDR_IN(&direccion_master, master->YAMA_IP,master->YAMA_PUERTO);

	if (connect(socket_master, (struct sockaddr*) &direccion_master,sizeof(struct sockaddr)) == -1) {
		log_error(logTrace, "Error al conectarse con YAMA");
	}


	while(1){

	}

	return EXIT_SUCCESS;
}

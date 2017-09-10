#include <stdio.h>
#include <stdlib.h>
#include "socket.h"
#include "serial.h"

#include<commons/log.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include<commons/collections/queue.h>
#include <commons/string.h>
#include <arpa/inet.h>


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
int socket_yama;
t_log * logTrace;
void handshake(int );

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


	struct sockaddr_in addr_yama;

	master = getConfigMaster(argv[1]);
	mostrarConfiguracion(master);
	crearLogger();

	socket_yama = crearSocket();
	inicializarSOCKADDR_IN(&addr_yama, master->YAMA_IP,master->YAMA_PUERTO);

	if (connect(socket_yama, (struct sockaddr*) &addr_yama,sizeof(struct sockaddr)) == -1) {
		log_error(logTrace, "Error al conectarse con YAMA");
	}
	handshake(socket_yama);
	//indico el archivo sobre el que deseo operar
	//quedo a la espera de indicaciones de yama
	//simulo que recibo una ip y un puerto de yama para conectarme con un worker.IP=127.0.0.1 PUERTO= 5050
	//pruebo conexion
	struct sockaddr_in addr_worker;
	//me conecto con un worker
	int	socket_worker = crearSocket();
	inicializarSOCKADDR_IN(&addr_worker,"127.0.0.1","5050");

	if(connect(socket_worker,(struct sockaddr*) &addr_worker,sizeof(struct sockaddr)) == -1){
			perror("Error en la conexion con Worker");
	}

	handshake(socket_worker);
	while(1);
	return EXIT_SUCCESS;
}

void handshake(int socket){
	 char handshake[26];
	 int cantBytes;
	 if((cantBytes = send(socket,"Yatpos-Master",sizeof("Yatpos-Master"),0)) <= 0) //envio credencial
	 {
	  perror("No pudo enviar!");
	  exit(1);
	 }
	 printf("envie %d \n",cantBytes);
	 if ((recv(socket,handshake,26,0)) <= 0)
	 {
	  perror("Mi cliente se esconectó");
	  exit(1);
	 }
	 handshake[26] = '\0';
	 printf("%s\n",handshake);
}



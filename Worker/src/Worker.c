#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <process.h>
#include <file.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <config.h>
#include <protocol.h>
#include <log.h>
#include <serial.h>

#include <commons/config.h>
#include <commons/log.h>
#include <commons/string.h>
#include <config.h>
#include <arpa/inet.h>

#define MAX_IP_LEN 16   // aaa.bbb.ccc.ddd -> son 15 caracteres, 16 contando un '\0'
#define MAX_PORT_LEN 6  // 65535 -> 5 digitos, 6 contando un '\0'
#define MAX_NOMBRE_NODO 5
#define MAX_RUTA 25
#define MAXIMO_TAMANIO_DATOS 256 //definiendo el tamanio maximo
#define MAXCONEXIONESLISTEN 10




#define RUTA "config_worker"

typedef struct{
	char * IP_FILESYSTEM,
		 * PUERTO_FILESYSTEM,
	     * NOMBRE_NODO,
		 * PUERTO_WORKER,
		 * PUERTO_DATANODE,
		 * RUTA_DATABIN;

}tWorker;

//t_log * logTrace;
typedef struct {
 int id;
 int tamanio;
} t_cabecera;
int socketEscuchandoMaster;
int socketCliente;


//funciones
void handshakeConMaster(int nuevoSocket, struct sockaddr_in remoteaddr);
void escucharPuertosMaster();
void creoHijos();
void configurarListener(int, int);

//void crearLogger() {
//   char *pathLogger = string_new();
//
//   char cwd[1024];
//
//   string_append(&pathLogger, getcwd(cwd, sizeof(cwd)));
//
//   string_append(&pathLogger, "/Worker_LOG.log");
//
//   char *logWorker = strdup("Worker_LOG.log");
//
//   logTrace = log_create(pathLogger, logWorker, false, LOG_LEVEL_INFO);
//
//   free(pathLogger);
//   free(logWorker);
//}

tWorker *getConfigWorker() {
	printf("Ruta del archivo de configuracion: %s\n", RUTA);
	tWorker *worker = malloc(sizeof(tWorker*));

	//t_config *workerConfig = config_create("/home/utnso/tp-2017-2c-YATPOS/Worker/src/config_worker");

	t_config *workerConfig = config_create(RUTA);

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


	tWorker * worker;
	worker = getConfigWorker();
	mostrarConfiguracion(worker);

	escucharPuertosMaster(worker);

	return EXIT_SUCCESS;
}

void escucharPuertosMaster(tWorker *worker) {
	 socklen_t clienteLen;
	 //Variables Sockets
	  struct sockaddr_in dirWorker;
	  struct sockaddr_in dirCliente;
	//////PREPARO EL SOCKET PARA ESCUCHAR CONEXIONES
	 inicializarSOCKADDR_IN(&dirWorker, "127.0.0.1", worker->PUERTO_WORKER); //IP="0" PARA QUE TOME MAQUINA LOCAL
	 socketEscuchandoMaster = crearSocket();
	 reutilizarSocket(socketEscuchandoMaster);
	 asignarDirecciones(socketEscuchandoMaster, (struct sockaddr *) &dirWorker);
	 configurarListener(socketEscuchandoMaster, MAXCONEXIONESLISTEN);
	 clienteLen = sizeof(dirCliente);
	 printf("Estoy escuchando conexiones \n");

	 //Acepto conexiones entrantes, validando que sean solo de tipo Master.
	 if ((socketCliente = accept(socketEscuchandoMaster,(struct sockaddr *) &dirCliente,(socklen_t *) &clienteLen)) < 0) {
	  perror("Error en la aceptacion \n");
	  close(socketEscuchandoMaster);
	  exit(0);
	 }
	 //QUEDA BLOQUEADO POR EL ACCEPT HASTA QUE ALGUIEN SE CONECTA
	 handshakeConMaster(socketCliente, dirCliente);
	 creoHijos();
	 while(1);
}


void configurarListener(int socket, int cantConexiones) {
	if (listen(socket, cantConexiones) == -1) {
		perror("listen");
		exit(1);
	}
}

void handshakeConMaster(int nuevoSocket, struct sockaddr_in remoteaddr){
		char buffer_select[MAXIMO_TAMANIO_DATOS];
		int cantidadBytes;
		strcpy(buffer_select,"");
		//cantidadBytes = recv(nuevoSocket, buffer_select, sizeof(buffer_select), 0);
		t_socket cli_sock = nuevoSocket;
		t_packet packet = protocol_receive(cli_sock);


//		printf("recibi %d \n",cantidadBytes);
//		printf("Credencial recibida: %d\n", *((int*)buffer_select));
		//!strcmp(buffer_select, "Yatpos-Master\0")
		if (packet.operation == OP_HANDSHAKE) { //Yatpos es la credencial que autoriza al proceso a seguir conectado
			printf("Yama: nueva conexion desde %s en socket %d\n", inet_ntoa(remoteaddr.sin_addr), nuevoSocket);
			socket_send_string(nuevoSocket,"Bienvenido a Worker!");
		} else {
			printf("El proceso que requirio acceso, no posee los permisos adecuados\n");
			socket_send_string(nuevoSocket,"No posee los permisos adecuados");
			close(nuevoSocket);
		}
 }

void creoHijos(){
	if ( fork() == 0 ) {
	/* Lógica del proceso HIJO */
	printf("Hola soy el hijo \n");
	//exit(0);
	}
	else {
	/* Lógica del proceso PADRE*/
		printf("Hola soy el proceso padre \n");
	}
}

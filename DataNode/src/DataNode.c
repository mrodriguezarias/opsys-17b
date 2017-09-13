#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <commons/config.h>
#include <commons/log.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <config.h>
#include <log.h>
#include <process.h>
#include <protocol.h>
#include <socket.h>
#include <string.h>
#include "socket.h"
#include "serial.h"

//Estructuras
typedef struct{
 char* IP_FILESYSTEM;
 char* NOMBRE_NODO;
 char* PUERTO_WORKER;
 char* PUERTO_DATANODE;
 char* RUTA_DATABIN;
}tinformacion;

typedef struct {
	int nroBloque;
} t_getBloque;

typedef struct {
	int nroBloque;
	char* datos; //COMO HARIAMOS PARA RECIBIR ESTA ESTRUCTURA? YA QUE LOS DATOS PUEDEN VARIAR A DIFERENCIA DEL VECTOR
} t_setBloque;

typedef struct {
 int id;
 int tamanio;
} t_cabecera;

t_socket socket_FileSystem;
t_log* logDataNode;

//Funciones
void handshakeConFS();
void listen_for_operations();
void connect_to_filesystem();
t_getBloque getBloque_unpack(t_serial);
t_setBloque setBloque_unpack(t_serial);
void getBloque_operation(int);
void setBloque_operation(int, char*);

int main(){
	logDataNode = log_create("logDataNode.log", "DataNode", true, LOG_LEVEL_TRACE);
	process_init(PROC_DATANODE);
	connect_to_filesystem();
	//listen_for_operations();
	return EXIT_SUCCESS;
}

void connect_to_filesystem(){
	t_socket socket = socket_connect(config_get("IP_FILESYSTEM"), config_get("PUERTO_DATANODE"));
	protocol_handshake(socket); //ACA DEBERIA IR HANDSHAKECONFS o esto funciona bien?
	//log_inform("Conectado a proceso FileSystem por socket %i", socket); //Este log es el log propio del Datanode o es uno general?
	socket_FileSystem = socket;
}


void handshakeConFS(){
	char handshake[26];

	if((send(socket_FileSystem,"Yatpos-DataNode",sizeof("Yatpos-DataNode"),0)) <= 0) //envio credencial
	{
		perror("No pudo enviar!");
		exit(1);
	}
	if ((recv(socket_FileSystem,handshake,26,0)) <= 0) //"Bienvenido al FileSystem!"
	{
		perror("El FileSystem se desconectó");
		exit(1);
	}
	handshake[26] = '\0';
	printf("%s\n",handshake);
}

void listen_for_operations(){
	while(1){
		t_packet packet = protocol_receive(socket_FileSystem);

		if(packet.sender == PROC_FILESYSTEM && packet.operation == GETBLOQUE){ //HABRIA QUE AGREGAR A LA LISTA DE OPERACIONES OP_AUTH_GETBLOQUE
			t_getBloque getBloque = getBloque_unpack(packet.content);
			getBloque_operation(getBloque.nroBloque);
		}else if(packet.sender == PROC_FILESYSTEM && packet.operation == SETBLOQUE){ //HABRIA QUE AGREGAR A LA LISTA DE OPERACIONES OP_AUTH_SETBLOQUE
			t_setBloque setBloque = setBloque_unpack(packet.content);
			setBloque_operation(setBloque.nroBloque, setBloque.datos);
		}

		free(packet.content.data);
	}
}

//ESTAS 2 DEBERIAN ESTAR EN COMUN O NO? YA QUE SOLO SE USAN EN DATANODE
t_getBloque getBloque_unpack(t_serial serial){
	t_getBloque getBloque;
	serial_unpack(serial, "scIlhhh", &getBloque.nroBloque);
	return getBloque;
}

t_setBloque setBloque_unpack(t_serial serial){
	t_setBloque setBloque;
	serial_unpack(serial, "scIlhhh", &setBloque.nroBloque, &setBloque.datos);
	return setBloque;
}

void getBloque_operation(int nroBloque){

}

void setBloque_operation(int nroBloque, char* datos){

}

//Adentro del main iba esto:
/*
	struct sockaddr_in addr_FileSystem;
	char* path = "/home/utnso/datanode_config";
	t_config* archivoConfig = config_create(path);
	logDataNode = log_create("logDataNode.log", "DataNode", true,LOG_LEVEL_TRACE);
	tinformacion informacion_socket;
	t_cabecera header;

	//VALIDACIONES

	if (archivoConfig == NULL) {
		log_error(logDataNode,"Archivo configuracion: error al intentar leer ruta");
		return 1;
	}
	if (!config_has_property(archivoConfig, "IP_FILESYSTEM")) {
		log_error(logDataNode, "Archivo configuracion: error al leer IP_FILESYSTEM");
		return 1;
	}
	if (!config_has_property(archivoConfig, "NOMBRE_NODO")) {
		log_error(logDataNode, "Archivo configuracion: error al leer NOMBRE_NODO");
	}
	if (!config_has_property(archivoConfig, "PUERTO_WORKER")) {
		log_error(logDataNode, "Archivo configuracion: error al leer PUERTO_WORKER");
	}
	if (!config_has_property(archivoConfig, "PUERTO_DATANODE")) {
		log_error(logDataNode, "Archivo configuracion: error al leer PUERTO_DATANODE");
	}
	if (!config_has_property(archivoConfig, "RUTA_DATABIN")) {
		log_error(logDataNode, "Archivo configuracion: error al leer RUTA_DATABIN");
	}
	//fin VALIDACIONES

	//configuracion del archivo configuracion
	informacion_socket.IP_FILESYSTEM = config_get_string_value(archivoConfig, "IP_FILESYSTEM");
	informacion_socket.NOMBRE_NODO = config_get_string_value(archivoConfig, "NOMBRE_NODO");
	informacion_socket.PUERTO_WORKER = config_get_string_value(archivoConfig, "PUERTO_WORKER");
	informacion_socket.PUERTO_DATANODE = config_get_string_value(archivoConfig, "PUERTO_DATANODE");
	informacion_socket.RUTA_DATABIN = config_get_string_value(archivoConfig, "RUTA_DATABIN");

	socket_FileSystem = crearSocket();
	inicializarSOCKADDR_IN(&addr_FileSystem,informacion_socket.IP_FILESYSTEM,informacion_socket.PUERTO_DATANODE);

	if(connect(socket_FileSystem,(struct sockaddr*) &addr_FileSystem,sizeof(struct sockaddr)) == -1){
		perror("Error en la conexion con el FileSystem");
	}

	handshakeConFS();

	//Aca comenzaria el ciclo de escucha de mensajes
	while(1){
		recv(socket_FileSystem,&header,sizeof(header),0);
		switch (header.id){
		case '1': //aca deberia ir como recibamos el opcion, queda a gusto del equipo :)
		break;
		}

	}
	*/

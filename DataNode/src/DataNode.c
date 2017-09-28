#include <config.h>
#include <data.h>
#include <log.h>
#include <mstring.h>
#include <process.h>
#include <protocol.h>
#include <serial.h>
#include <socket.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

typedef struct {
	const char* nombreNodo;
	int total;
} Nodo;

t_socket socket_FileSystem;

//Funciones
void listen_for_operations();
void connect_to_filesystem();
t_getBloque getBloque_unpack(t_serial);
t_setBloque setBloque_unpack(t_serial);
void getBloque_operation(int);
void setBloque_operation(int, char*);
void inicializarNodo();
t_serial nodo_pack(Nodo infoNodo);
void handshakeconFS(int);

int main() {
	process_init(PROC_DATANODE);
	data_open(config_get("RUTA_DATABIN"), mstring_toint(config_get("DATABIN_SIZE")));
	connect_to_filesystem();
	//listen_for_operations();
	data_close();
	return EXIT_SUCCESS;
}

void connect_to_filesystem(){
	t_socket socket = socket_connect(config_get("IP_FILESYSTEM"), config_get("PUERTO_DATANODE"));
	protocol_handshake(socket);
	printf("socket%d \n",socket);
	//handshakeconFS(socket);
	log_inform("Conectado a proceso FileSystem por socket %i", socket);
	socket_FileSystem = socket;
	inicializarNodo();
	while(1);
}
void handshakeconFS(int socket){
 char buffer_select[256];
 strcpy(buffer_select, "");
 strcpy(buffer_select, "Yatpos-DataNode");
 send(socket, &buffer_select, sizeof(buffer_select), 0);
 recv(socket, buffer_select, 26, 0);
}

void listen_for_operations(){
	while(1){
		t_packet packet = protocol_receive(socket_FileSystem);

		if(packet.sender == PROC_FILESYSTEM && packet.operation == OP_GETBLOQUE){ //HABRIA QUE AGREGAR A LA LISTA DE OPERACIONES OP_AUTH_GETBLOQUE
			t_getBloque getBloque = getBloque_unpack(packet.content);
			getBloque_operation(getBloque.nroBloque);
		}else if(packet.sender == PROC_FILESYSTEM && packet.operation == OP_SETBLOQUE){ //HABRIA QUE AGREGAR A LA LISTA DE OPERACIONES OP_AUTH_SETBLOQUE
			t_setBloque setBloque = setBloque_unpack(packet.content);
			setBloque_operation(setBloque.nroBloque, setBloque.datos);
		}

		free(packet.content.data);
	}
}

t_getBloque getBloque_unpack(t_serial serial){
	t_getBloque getBloque;
	serial_unpack(serial, "i", &getBloque.nroBloque);
	return getBloque;
}

t_setBloque setBloque_unpack(t_serial serial){
	t_setBloque setBloque;
	serial_unpack(serial, "is", &setBloque.nroBloque, &setBloque.datos);
	return setBloque;
}

void getBloque_operation(int nroBloque){

}

void setBloque_operation(int nroBloque, char* datos){

}

void inicializarNodo(){
	int operacion;
	//Recibo la operacion
	recv(socket_FileSystem, &operacion, sizeof(int), 0);
	printf("entre a ininicializar nodo \n");
	if(operacion == REGISTRARNODO){
		Nodo infoNodo;
		infoNodo.nombreNodo = config_get("NOMBRE_NODO");
		infoNodo.total = data.size;
		//Empaqueto
		t_serial packed_nodo = nodo_pack(infoNodo);
		t_packet packet = protocol_packet(REGISTRARNODO, packed_nodo);
		//Envio
		protocol_send(packet, socket_FileSystem);
	}
}

t_serial nodo_pack(Nodo infoNodo) {
	return serial_pack("si", infoNodo.nombreNodo, infoNodo.total);
}


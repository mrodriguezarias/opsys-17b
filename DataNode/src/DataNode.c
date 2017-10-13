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
#include <unistd.h>
#include <thread.h>

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

static t_socket fs_socket;
static const char *node_name;

static void load_name_from_args(int argc, char **argv);
static void connect_to_filesystem(void);
static void send_node_info(void);
static void handle_request(t_packet packet);
static void terminate(void);

void listen_for_operations();
t_getBloque getBloque_unpack(t_serial*);
t_setBloque setBloque_unpack(t_serial*);
void getBloque_operation(int);
void setBloque_operation(int, char*);
void inicializarNodo();
t_serial *nodo_pack(Nodo infoNodo);

// ========== Funciones públicas ==========

int main(int argc, char *argv[]) {
	process_init();
	thread_signal_capture(SIGINT, terminate);

	data_open(config_get("RUTA_DATABIN"), mstring_toint(config_get("DATABIN_SIZE")));
	connect_to_filesystem();

	load_name_from_args(argc, argv);
	send_node_info();

	while(true) {
		t_packet packet = protocol_receive_packet(fs_socket);
		if(packet.operation == OP_UNDEFINED) {
			fprintf(stderr, "\33[2K\rConexión con el FileSystem terminada\n");
			break;
		}

		handle_request(packet);
	}

	terminate();
	return EXIT_SUCCESS;
}

// ========== Funciones privadas ==========

/**
 * Toma opcionalmente el nombre del nodo pasado como argumento.
 * Para facilitar las pruebas y evitar tener que estar cambiando el archivo config.
 */
static void load_name_from_args(int argc, char **argv) {
	node_name = argc == 2 ? mstring_duplicate(argv[1]) : config_get("NOMBRE_NODO");
}

static void connect_to_filesystem() {
	t_socket socket = socket_connect(config_get("IP_FILESYSTEM"), config_get("PUERTO_DATANODE"));
	protocol_send_handshake(socket);
	log_inform("Conectado al FileSystem por socket %i", socket);
	fs_socket = socket;
}

static void send_node_info() {
	t_serial *node_info = serial_pack("si", node_name, data_blocks());
	t_packet packet = protocol_packet(OP_NODE_INFO, node_info);
	protocol_send_packet(packet, fs_socket);
	serial_destroy(node_info);

	int response = protocol_receive_response(fs_socket);
	if(response != 0) {
		fprintf(stderr, "El FileSystem rechazó la conexión\n");
		terminate();
	}
}

static void handle_request(t_packet packet) {
	if(packet.operation == OP_SET_BLOCK) {
		int blockno;
		t_serial *block;
		serial_unpack(packet.content, "ix", &blockno, &block);
		printf("Solicitud de escritura de bloque #%d\n", blockno);
		printf("Tamaño: %d\n", block->size);
		printf("Contenido: %s\n", mstring_copy(block->data, 0, block->size));
		serial_destroy(block);
	} else if (packet.operation == OP_GET_BLOCK) {
		// TODO: Get block
	}
}

static void terminate() {
	socket_close(fs_socket);
	data_close();
	log_inform("Desconectado. Proceso terminado.");
	exit(EXIT_SUCCESS);
}

void listen_for_operations(){
	while(1){
		t_packet packet = protocol_receive_packet(fs_socket);

		if(packet.sender == PROC_FILESYSTEM && packet.operation == OP_GETBLOQUE){ //HABRIA QUE AGREGAR A LA LISTA DE OPERACIONES OP_AUTH_GETBLOQUE
			t_getBloque getBloque = getBloque_unpack(packet.content);
			getBloque_operation(getBloque.nroBloque);
		}else if(packet.sender == PROC_FILESYSTEM && packet.operation == OP_SETBLOQUE){ //HABRIA QUE AGREGAR A LA LISTA DE OPERACIONES OP_AUTH_SETBLOQUE
			t_setBloque setBloque = setBloque_unpack(packet.content);
			setBloque_operation(setBloque.nroBloque, setBloque.datos);
		}

		serial_destroy(packet.content);
	}
}

t_getBloque getBloque_unpack(t_serial *serial){
	t_getBloque getBloque;
	serial_unpack(serial, "i", &getBloque.nroBloque);
	return getBloque;
}

t_setBloque setBloque_unpack(t_serial *serial){
	t_setBloque setBloque;
	serial_unpack(serial, "is", &setBloque.nroBloque, &setBloque.datos);
	return setBloque;
}

void getBloque_operation(int nroBloque){

}

void setBloque_operation(int nroBloque, char* datos){

}

void inicializarNodo(){
	int operacion = 0;
	//Recibo la operacion
	recv(fs_socket, &operacion, sizeof(operacion), 0);
	if(operacion == REGISTRARNODO){
		Nodo infoNodo;
		infoNodo.nombreNodo = config_get("NOMBRE_NODO");
		infoNodo.total = (data_size()/1048576); //pasado a megas.
		//Empaqueto
		t_serial *packed_nodo = nodo_pack(infoNodo);
		t_packet packet = protocol_packet(REGISTRARNODO, packed_nodo);
		//Envio
		protocol_send_packet(packet, fs_socket);
	}
}

t_serial *nodo_pack(Nodo infoNodo) {
	return serial_pack("si", infoNodo.nombreNodo, infoNodo.total);
}


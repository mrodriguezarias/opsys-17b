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

static t_socket fs_socket = -1;

static void connect_to_filesystem(void);
static void send_node_info(void);
static void handle_request(t_packet request);
static void terminate(void);

// ========== Funciones públicas ==========

int main(int argc, char *argv[]) {
	process_init();
	thread_signal_capture(SIGINT, terminate);
	data_open(config_get("RUTA_DATABIN"), mstring_toint(config_get("DATABIN_SIZE")));

	start:
	connect_to_filesystem();
	send_node_info();

	while(true) {
		t_packet packet = protocol_receive_packet(fs_socket);
		if(packet.operation == OP_UNDEFINED) {
			fprintf(stderr, "\33[2K\rConexión con el FileSystem terminada\n");
			socket_close(fs_socket);
			goto start;
		}

		handle_request(packet);
	}

	terminate();
	return EXIT_SUCCESS;
}

// ========== Funciones privadas ==========

static void connect_to_filesystem() {
	t_socket socket = socket_connect(config_get("IP_FILESYSTEM"), config_get("PUERTO_FILESYSTEM"));
	if(socket == -1) puts("Esperando conexión del FileSystem...");
	while(socket == -1) {
		thread_sleep(500);
		socket = socket_connect(config_get("IP_FILESYSTEM"), config_get("PUERTO_FILESYSTEM"));
	}

	protocol_send_handshake(socket);
	log_print("Conectado al FileSystem por socket %i", socket);
	fs_socket = socket;
}

static void send_node_info() {
	t_serial *node_info = serial_pack("sis", config_get("NOMBRE_NODO"), data_blocks(), config_get("PUERTO_WORKER"));
	t_packet packet = protocol_packet(OP_NODE_INFO, node_info);
	protocol_send_packet(packet, fs_socket);
	serial_destroy(node_info);
	int response = protocol_receive_response(fs_socket);
	if(response != 0) {
		fprintf(stderr, "El FileSystem rechazó la conexión\n");
		terminate();
	}
}

static void handle_request(t_packet request) {
	if(request.operation == OP_PING) return;
	if(request.operation != OP_REQUEST_BLOCK) {
		log_report("Operación inválida. Código de operación: %i", request.operation);
		return;
	}
	int blockno, receiving;
	serial_unpack(request.content, "ii", &blockno, &receiving);
	if(receiving) {
		log_print("Solicitud de escritura de bloque #%i", blockno);
		t_packet packet = protocol_receive_packet(fs_socket);
		if(packet.operation != OP_SEND_BLOCK) {
			log_report("Se esperaba recibir un bloque pero se recibió otra cosa");
			return;
		}
		data_set(blockno, packet.content->data);
		serial_destroy(packet.content);
	} else {
		log_print("Solicitud de lectura de bloque #%i", blockno);
		t_serial *block = serial_create(data_get(blockno), BLOCK_SIZE);
		t_packet response = protocol_packet(OP_SEND_BLOCK, block);
		protocol_send_packet(response, fs_socket);
		free(block);
	}
}

static void terminate() {
	printf("\33[2K\r");
	if(fs_socket != -1) socket_close(fs_socket);
	data_close();
	log_inform("Desconectado. Proceso terminado.");
	process_term();
	exit(EXIT_SUCCESS);
}


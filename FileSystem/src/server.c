#include "server.h"
#include <stddef.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <config.h>
#include <protocol.h>
#include "estructuras.h"
#include <mlist.h>
#include <thread.h>
#include <protocol.h>
#include "nodelist.h"
#include <log.h>
#include "FileSystem.h"

#define MAXIMO_TAMANIO_DATOS 256 //definiendo el tamanio maximo

typedef struct {
	int id;
	int tamanio;
} t_cabecera;

static t_node *receive_node_info(t_socket socket);
static void datanode_listener(void);
static void datanode_handler(t_node *node);

// ========== Funciones públicas ==========

void server() {
	thread_create(datanode_listener, NULL);
}

// ========== Funciones privadas ==========

static t_node *receive_node_info(t_socket socket) {
	t_packet packet = protocol_receive_packet(socket);
	if(packet.operation != OP_NODE_INFO) return NULL;
	char *name;
	int blocks;
	serial_unpack(packet.content, "si", &name, &blocks);
	t_node *node = nodelist_find(name);
	if(node == NULL && fs.formatted) {
		log_inform("El nodo %s se quiso conectar pero el FS ya estaba formateado", name);
	} else if(node != NULL && node->available) {
		node = NULL;
		log_inform("El nodo %s se quiso conectar pero ya estaba activo en el FS", name);
	} else {
		node = nodelist_add(name, blocks, thread_self());
	}
	free(name);
	return node;
}

static void datanode_listener() {
	t_socket sv_sock = socket_init(NULL, config_get("PUERTO_DATANODE"));

	while(thread_active()) {
		t_socket cli_sock = socket_accept(sv_sock);

		t_packet packet = protocol_receive_packet(cli_sock);
		if(packet.operation != OP_HANDSHAKE || packet.sender != PROC_DATANODE) {
			socket_close(cli_sock);
			break;
		}

		t_node *node = receive_node_info(cli_sock);
		if(node == NULL) {
			protocol_send_response(cli_sock, RESPONSE_ERROR);
			socket_close(cli_sock);
			continue;
		}

		protocol_send_response(cli_sock, RESPONSE_OK);
		log_inform("DataNode del nodo %s conectado", node->name);

		node->socket = cli_sock;
		node->handler = thread_create(datanode_handler, node);
		node->available = true;
	}

	socket_close(sv_sock);
}

static void datanode_handler(t_node *node) {
	while(thread_active()) {
		int blockno = (int) thread_receive();
		t_serial *block = thread_receive();
		if(block != NULL) {
			t_serial *serial = serial_pack("i", blockno);
			t_packet packet = protocol_packet(OP_SET_BLOCK, serial);
			protocol_send_packet(packet, node->socket);
			serial_destroy(serial);

			packet = protocol_packet(OP_SET_BLOCK, block);
			protocol_send_packet(packet, node->socket);
			serial_destroy(block);
		}
	}

	node->available = false;
	node->handler = NULL;
	log_inform("DataNode del nodo %s desconectado", node->name);
}

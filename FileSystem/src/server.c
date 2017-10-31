#include "server.h"

#include <config.h>
#include <log.h>
#include <process.h>
#include <protocol.h>
#include <serial.h>
#include <socket.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <thread.h>
#include <data.h>
#include <path.h>
#include <string.h>
#include <commons/string.h>

#include "FileSystem.h"
#include "nodelist.h"
#include "filetable.h"

static t_node *receive_node_info(t_socket socket);
static void node_listener(void);
static void yama_listener(void);
static void datanode_handler(t_node *node);
static void yama_handler(t_socket socket);

// ========== Funciones públicas ==========

void server() {
	thread_create(node_listener, NULL);
	thread_create(yama_listener, NULL);
}

t_nodeop *server_nodeop(int opcode, int blockno, t_serial *block) {
	t_nodeop *op = malloc(sizeof(t_nodeop));
	op->opcode = opcode;
	op->blockno = blockno;
	op->block = block;
	return op;
}

// ========== Funciones privadas ==========

static t_node *receive_node_info(t_socket socket) {
	t_packet packet = protocol_receive_packet(socket);
	if(packet.operation != OP_NODE_INFO) return NULL;
	char *name, *worker_port;
	int blocks;
	serial_unpack(packet.content, "sis", &name, &blocks, &worker_port);
	t_node *node = nodelist_find(name);

	if(node == NULL && fs.formatted) {
		log_inform("El nodo %s se quiso conectar pero el FS ya estaba formateado", name);
	} else if(nodelist_active(node)) {
		node = NULL;
		log_inform("El nodo %s se quiso conectar pero ya estaba activo en el FS", name);
	} else {
		node = nodelist_add(name, blocks);
		node->worker_port = string_duplicate(worker_port);
	}
	free(name);
	return node;
}

static void node_listener() {
	t_socket sv_sock = socket_init(NULL, config_get("PUERTO_NODO"));

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

		char *ip = socket_address(cli_sock);
		char *port = socket_port(cli_sock);
		log_inform("Nodo %s conectado desde %s:%s", node->name, ip, port);
		free(ip);
		free(port);

		node->socket = cli_sock;
		node->handler = thread_create(datanode_handler, node);
	}

	socket_close(sv_sock);
}

static void yama_listener() {
	t_socket sv_sock = socket_init(NULL, config_get("PUERTO_YAMA"));

	while(thread_active()) {
		t_socket yama_socket = socket_accept(sv_sock);

		t_packet packet = protocol_receive_packet(yama_socket);
		if(packet.operation != OP_HANDSHAKE || packet.sender != PROC_YAMA) {
			socket_close(yama_socket);
			continue;
		}

		if(!filetable_stable()){
			log_inform("Filesystem no estable. Se rechaza conexión de YAMA");
			protocol_send_response(yama_socket, RESPONSE_ERROR);
			socket_close(yama_socket);
			continue;
		}

		log_inform("Yama conectado en socket: %d", yama_socket);
		protocol_send_response(yama_socket, RESPONSE_OK);
		yama_handler(yama_socket);
	}

	socket_close(sv_sock);
}

static void yama_handler(t_socket yama_socket) {

	while(thread_active()) {

		t_packet packet = protocol_receive_packet(yama_socket);
		if (packet.operation == OP_REQUEST_FILE_INFO) {
				log_inform("Receive OP_REQUEST_FILE_INFO");

				char* file_request;
				serial_unpack(packet.content, "s", &file_request);
				packet = protocol_packet(OP_NODES_ACTIVE_INFO, nodelist_active_pack());
				protocol_send_packet(packet, yama_socket);
				log_inform("Send OP_NODES_ACTIVE_INFO");

				t_yfile * yfile = filetable_find(file_request);
				if(yfile == NULL){
					packet = protocol_packet(OP_ARCHIVO_INEXISTENTE, serial_pack("s", file_request));
					protocol_send_packet(packet, yama_socket);
					log_inform("Send OP_ARCHIVO_INEXISTENTE %s", file_request);
				}else{
					packet = protocol_packet(OP_ARCHIVO_NODES, yfile_pack(yfile));
					protocol_send_packet(packet, yama_socket);
					log_inform("Send OP_ARCHIVO_NODES");
				}
		}
		serial_destroy(packet.content);
	}
}


static void datanode_handler(t_node *node) {
	t_nodeop *op;
	while(op = thread_receive(), thread_active()) {
		if(op->opcode == NODE_PING) {
			free(op);
			protocol_send_packet(protocol_packet(OP_PING, NULL), node->socket);
			bool alive = protocol_send_packet(protocol_packet(OP_PING, NULL), node->socket);
			thread_respond((void*)alive);
			if(alive) continue;
			else break;
		}

		t_serial *serial = serial_pack("ii", op->blockno, op->opcode == NODE_SEND);
		t_packet packet = protocol_packet(OP_REQUEST_BLOCK, serial);
		protocol_send_packet(packet, node->socket);
		serial_destroy(serial);

		if(op->opcode == NODE_SEND) {
			log_inform("Enviando bloque %d a nodo %s", op->blockno, node->name);
			packet = protocol_packet(OP_SEND_BLOCK, op->block);
			protocol_send_packet(packet, node->socket);
			serial_destroy(op->block);
		} else if(op->opcode == NODE_RECV) {
			log_inform("Recibiendo bloque %d de nodo %s", op->blockno, node->name);
			packet = protocol_receive_packet(node->socket);
			if(packet.operation != OP_SEND_BLOCK) {
				log_report("Se esperaba recibir un bloque pero se recibió otra cosa");
			} else {
				filetable_writeblock(op->blockno, packet.content->data);
				serial_destroy(packet.content);
			}
		}
		free(op);
	}

	node->handler = NULL;
	log_inform("DataNode del nodo %s desconectado", node->name);
}

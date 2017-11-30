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
static void worker_handler(t_socket worker_socket);
static void datanode_handler(t_node *node);
static void yama_handler(t_socket socket);

// ========== Funciones públicas ==========

void server() {
	thread_create(node_listener, NULL);
	thread_create(yama_listener, NULL);
}

t_nodeop *server_nodeop(int opcode, int blockno, void *block) {
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
	free(worker_port);
	return node;
}

static void node_listener() {
	t_socket sv_sock = socket_init(NULL, config_get("PUERTO_NODO"));

	while(thread_active()) {
		t_socket cli_sock = socket_accept(sv_sock);

		t_packet packet = protocol_receive_packet(cli_sock);
		serial_destroy(packet.content);

		if(packet.operation != OP_HANDSHAKE) {
			socket_close(cli_sock);
			break;
		}

		char *ip = socket_address(cli_sock);
		char *port = socket_port(cli_sock);

		if (packet.sender == PROC_DATANODE) {
			t_node *node = receive_node_info(cli_sock);
			if(node == NULL) {
				protocol_send_response(cli_sock, RESPONSE_ERROR);
				socket_close(cli_sock);
				continue;
			}

			protocol_send_response(cli_sock, RESPONSE_OK);

			log_inform("Nodo %s conectado desde %s:%s", node->name, ip, port);

			node->socket = cli_sock;
			node->handler = thread_create(datanode_handler, node);
		} else if (packet.sender == PROC_WORKER) {
			protocol_send_response(cli_sock, RESPONSE_OK);

			log_inform("Worker conectado desde %s:%s", ip, port);

			thread_create(worker_handler, (void *) cli_sock);
		}

		free(ip);
		free(port);
	}

	socket_close(sv_sock);
}

static void worker_handler(t_socket worker_socket) {
	t_packet packet = protocol_receive_packet(worker_socket);
	char *buffer, *ypath;
	int size;

	if (packet.content != NULL && packet.operation == OP_INICIAR_ALMACENAMIENTO) {
		log_inform("OP_INICIAR_ALMACENAMIENTO");
		serial_unpack(packet.content, "ssi", &buffer, &ypath, &size);

		if(filetable_contains(ypath)) {
			log_inform("El archivo ya existe");
			protocol_send_response(worker_socket, RESPONSE_ERROR);
			free(buffer);
			free(ypath);
		} else {
			mstring_format(&ypath, "%s", path_create(PTYPE_YAMA, ypath));

			t_file* file = file_create(path_name(ypath));
			fwrite(buffer, sizeof(char), size, file_pointer(file));
			free(buffer);

			char *path = mstring_duplicate(file_path(file));
			file_close(file);

			char *dir = path_dir(ypath);

			filetable_cpfrom(path, dir);
			free(dir);

			path_remove(path);
			free(path);

			if(filetable_contains(ypath)) {
				log_inform("Archivo almacenado");
				protocol_send_response(worker_socket, RESPONSE_OK);
			} else {
				log_inform("Espacio insuficiente para almacenar archivo");
				protocol_send_response(worker_socket, RESPONSE_ERROR);
			}
			free(ypath);
		}
	} else {
		log_inform("OP_UNDEFINED");
		protocol_send_response(worker_socket, RESPONSE_ERROR);
	}

	socket_close(worker_socket);
	thread_exit(0);
}

static void yama_listener() {
	t_socket sv_sock = socket_init(NULL, config_get("PUERTO_YAMA"));

	while(thread_active()) {
		t_socket yama_socket = socket_accept(sv_sock);

		t_packet packet = protocol_receive_packet(yama_socket);
		serial_destroy(packet.content);

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

			char *file_request;
			serial_unpack(packet.content, "s", &file_request);

			t_serial *active_nodes = nodelist_active_pack();
			packet = protocol_packet(OP_NODES_ACTIVE_INFO, active_nodes);
			protocol_send_packet(packet, yama_socket);
			serial_destroy(active_nodes);
			log_inform("Send OP_NODES_ACTIVE_INFO");

			t_yfile * yfile = filetable_find(file_request);
			if(yfile == NULL) {
				t_serial *pfreq = serial_pack("s", file_request);
				packet = protocol_packet(OP_ARCHIVO_INEXISTENTE, pfreq);
				protocol_send_packet(packet, yama_socket);
				serial_destroy(pfreq);
				log_inform("Send OP_ARCHIVO_INEXISTENTE %s", file_request);
			} else {
				t_serial *packed_file = yfile_pack(yfile);
				packet = protocol_packet(OP_ARCHIVO_NODES, packed_file);
				protocol_send_packet(packet, yama_socket);
				serial_destroy(packed_file);
				log_inform("Send OP_ARCHIVO_NODES");
			}

			free(file_request);
		} else {
			serial_destroy(packet.content);
			break;
		}
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
			t_serial *block = serial_create(op->block, BLOCK_SIZE);
			packet = protocol_packet(OP_SEND_BLOCK, block);
			protocol_send_packet(packet, node->socket);
			free(block);
		} else {
			log_inform("Recibiendo bloque %d de nodo %s", op->blockno, node->name);
			packet = protocol_receive_packet(node->socket);
			if(packet.operation != OP_SEND_BLOCK) {
				log_report("Se esperaba recibir un bloque pero se recibió otra cosa");
			} else if(op->opcode == NODE_RECV) {
				filetable_writeblock(node->name, op->blockno, packet.content->data);
			}  else if(op->opcode == NODE_RECV_BLOCK) {
				thread_respond((void*)packet.content->data);
			}
			serial_destroy(packet.content);
		}
		free(op);
	}

	node->handler = NULL;
	log_inform("DataNode del nodo %s desconectado", node->name);
}

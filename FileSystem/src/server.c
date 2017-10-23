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

#include "FileSystem.h"
#include "nodelist.h"
#include "filetable.h"

static struct {
	mutex_t *mut;
	int done;
	int total;
	t_file *fp;
	void *map;
} cfile;

static t_node *receive_node_info(t_socket socket);
static void datanode_listener(void);
static void yama_listener(void);
static void datanode_handler(t_node *node);
static void yama_handler();
static void update_current_file(void);

// ========== Funciones públicas ==========

void server_start() {
	cfile.mut = thread_mutex_create();
	thread_create(datanode_listener, NULL);
	thread_create(yama_listener, NULL);
}

t_nodeop *server_nodeop(int opcode, int blockno, t_serial *block) {
	t_nodeop *op = malloc(sizeof(t_nodeop));
	op->opcode = opcode;
	op->blockno = blockno;
	op->block = block;
	return op;
}

void server_set_current_file(t_yfile *file) {
	cfile.done = 0;
	cfile.total = mlist_length(file->blocks);
	path_truncate("metadata/blocks", cfile.total * BLOCK_SIZE);
	cfile.fp = file_open("metadata/blocks");
	cfile.map = file_map(cfile.fp);
}

void server_end() {
	thread_mutex_destroy(cfile.mut);
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
	} else if(nodelist_active(node)) {
		node = NULL;
		log_inform("El nodo %s se quiso conectar pero ya estaba activo en el FS", name);
	} else {
		node = nodelist_add(name, blocks);
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
			break;
		}

		if(!fs.formatted){
			log_inform("Filesystem no estable. Se rechaza conexión de YAMA");
			socket_close(yama_socket);
			break;
		}

		log_inform("Yama conectado en socket: %d", yama_socket);

		yama_handler();
	}

	socket_close(sv_sock);
}

static void yama_handler() {

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
				continue;
			}
			memcpy(cfile.map + op->blockno * BLOCK_SIZE, packet.content->data, BLOCK_SIZE);
			serial_destroy(packet.content);
			update_current_file();
		}
		free(op);
	}

	node->handler = NULL;
	log_inform("DataNode del nodo %s desconectado", node->name);
}

static void update_current_file() {
	bool file_done = false;
	thread_mutex_lock(cfile.mut);
	cfile.done++;
	file_done = cfile.done == cfile.total;
	thread_mutex_unlock(cfile.mut);

	if(!file_done) return;
	file_unmap(cfile.fp, cfile.map);
	file_close(cfile.fp);
	thread_resume(thread_main());
}

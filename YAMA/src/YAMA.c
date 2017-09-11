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

#define MAXIMO_TAMANIO_DATOS 256 //definiendo el tamanio maximo
int SocketBuscado_GLOBAL = 0;

typedef struct{
	char* FS_IP;
	char* FS_PUERTO;
	char* RETARDO_PLANIFICACION;
	char* ALGORITMO_BALANCEO;
	char* MASTER_PUERTO;
}tinformacion;

typedef struct {
	int id;
	int tamanio;
} t_cabecera;
int socket_FileSystem;

struct {
	t_socket fs_socket;
} yama;

void connect_to_filesystem(void);
void listen_to_master(void);

int main() {
	process_init(PROC_YAMA);
	//	connect_to_filesystem();
	listen_to_master();
	return EXIT_SUCCESS;
}

void connect_to_filesystem() {
	t_socket socket = socket_connect(config_get("FS_IP"), config_get("FS_PUERTO"));
	protocol_handshake(socket);
	logif("Conectado a proceso FileSystem por socket %i", socket);
	yama.fs_socket = socket;
}

void listen_to_master() {
	t_socket sv_sock = socket_init(NULL, config_get("MASTER_PUERTO"));
	t_fdset sockets = socket_set_create();
	socket_set_add(sv_sock, &sockets);

	while(true) {
		t_fdset selected = socket_select(sockets);

		for(t_socket sock = 3; sock <= sockets.max; sock++) {
			if(!socket_set_contains(sock, &selected)) continue;
			if(sock == sv_sock) {
				t_socket cli_sock = socket_accept(sv_sock);
				t_packet packet = protocol_receive(cli_sock);
				if(packet.operation == OP_HANDSHAKE && packet.sender == PROC_MASTER) {
					socket_set_add(cli_sock, &sockets);
					logif("Conectado proceso Master por socket %i", cli_sock);
				} else {
					socket_close(cli_sock);
				}
			} else {
//				t_packet packet = protocol_receive(sock);
//				if(packet.operation == OP_UNDEFINED) {
//					socket_close(sock);
//					socket_set_remove(sock, &sockets);
//					continue;
//				}

				char *string = socket_receive_string(sock);
				if(string) {
					printf("Recibido: %s\n", string);
				} else {
					logif("Desconectado proceso Master de socket %i", sock);
					socket_close(sock);
					socket_set_remove(sock, &sockets);
				}
			}
		}
	}
}

#include "Master.h"

#include <process.h>
#include <protocol.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "connection.h"
#include "manejadores.h"

// Esto es temporal, para probar la conexión con el Worker
#define WORKER_IP "127.0.0.1"
#define WORKER_PORT "5050"

t_master master;

int main(int argc, char *argv[]) {
	if(argc < 2) {
		puts("Uso: master archivo_a_procesar");
		return EXIT_SUCCESS;
	}

	process_init();
	connect_to_yama();
	request_job_for_file(argv[1]);
//	return 0;
	t_packet packet;
	do {
		packet = protocol_receive_packet(master.yama_socket);
		manejador_yama(packet,argv[1],argv[2],argv[3]);
	} while(true);
	terminate();
	return EXIT_SUCCESS;
}

void terminate() {
	socket_close(master.yama_socket);
	socket_close(master.worker_socket);
}

#include "Master.h"
#include "funcionesMaster.h"

#include <process.h>
#include <protocol.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "connection.h"
#include "manejadores.h"

int main(int argc, char *argv[]) {
	if(argc < 5) {
		puts("Faltan argumentos");
		return EXIT_SUCCESS;
	}

	init(argv);

	hilo_node_drop = thread_create(node_drop, NULL);

	do {
		t_packet paquete = protocol_receive_packet(yama_socket);
		manejador_yama(paquete);
	}while(job_active);


	terminate();
	return EXIT_SUCCESS;
}

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
	t_packet packet;
	do {
		packet = protocol_receive_packet(yama_socket);
		manejador_yama(packet);
	} while(job_active);
	terminate();
	return EXIT_SUCCESS;
}

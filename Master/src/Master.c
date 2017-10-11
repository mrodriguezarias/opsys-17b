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
	if(argc < 5) {
		puts("Faltan argumentos");
		return EXIT_SUCCESS;
	}

	job.script_transf = string_duplicate(argv[1]);
	job.script_reduc = string_duplicate(argv[2]);
	job.arch = string_duplicate(argv[3]);
	job.arch_result = string_duplicate(argv[4]);

	process_init();
	connect_to_yama();
	request_job_for_file(job.arch);

	t_packet packet;
	do {
		packet = protocol_receive_packet(master.yama_socket);
		manejador_yama(packet);
	} while(true);
	terminate();
	return EXIT_SUCCESS;
}

void terminate() {
	socket_close(master.yama_socket);
	socket_close(master.worker_socket);
}

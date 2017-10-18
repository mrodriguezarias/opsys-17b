#include "Master.h"

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
	job_init = get_current_time();

	job.path_transf = string_duplicate(argv[1]);
	job.path_reduc = string_duplicate(argv[2]);
	job.arch = string_duplicate(argv[3]);
	job.arch_result = string_duplicate(argv[4]);
	cargar_scripts(job.path_transf, job.path_reduc);
	hilos = mlist_create();
	pthread_mutex_init(&mutex_hilos, NULL);

	tareasParalelo.total = 0;
	tareasParalelo.transf = 0;
	tareasParalelo.reducc = 0;

	process_init();
	connect_to_yama();
	request_job_for_file(job.arch);
	job_active = true;

	t_packet packet;
	do {
		packet = protocol_receive_packet(yama_socket);
		manejador_yama(packet);
	} while(job_active);
	terminate();
	return EXIT_SUCCESS;
}

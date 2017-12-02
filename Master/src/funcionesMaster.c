#include "funcionesMaster.h"
#include "mstring.h"

void kill_thread(t_hilos* hilo){
	hilo->active = false;
	thread_kill(hilo->hilo);
	thread_sem_signal(sem);
}

void node_drop(){
	while(thread_active()){
		char* data = thread_receive();
		bool getSender(t_hilos* hilo){
			return hilo->active && hilo->hilo == thread_sender();
		}
		pthread_mutex_lock(&mutex_hilos);
		t_hilos* hilo_sender = mlist_find(hilos, getSender);
		if(hilo_sender != NULL && hilo_sender->active && !mstring_isempty(data)){
			bool getNodeDrop(t_hilos* hilo){
				return hilo->active &&
						mstring_equali(data, hilo->nodo) &&
						hilo->hilo != hilo_sender->hilo;
			}
			mlist_t* hilos_drop = mlist_filter(hilos, getNodeDrop);
			mlist_traverse(hilos_drop, kill_thread);
			for(int i = 0; i < mlist_length(hilos_drop); i++){
				t_hilos* hilo_drop_node = mlist_get(hilos_drop, i);
				log_report("Finalización del hilo %d por caída del nodo: %s",
						hilo_drop_node->hilo,
						hilo_drop_node->nodo);
			}

			thread_resume(hilo_sender->hilo);
		}
		pthread_mutex_unlock(&mutex_hilos);
	}
	thread_exit(0);
}

void actualizar_hilo(int response){
	bool getHilo(t_hilos* thread){
		return (thread->active && thread->hilo == thread_self());
	}
	t_hilos* hilo = mlist_find(hilos,getHilo);
	hilo->active = false;
	hilo->result = response;
}

bool enviar_operacion_worker(int operacion, t_socket socket, t_serial* serial_worker) {
	t_packet worker = protocol_packet(operacion,
			serial_worker);
	bool result = protocol_send_packet(worker, socket);
	serial_destroy(serial_worker);
	return result;
}

void response_worker(t_socket socket, int* response) {
	int operacion = protocol_receive_response(socket);
	printf("operacion: %d, , socket: %d  \n",operacion,socket);
	if (operacion != RESPONSE_OK) {
		printf("recibi operacion indefinida de worker \n");
		*response = 1; //iria -1
	} else {
		*response = operacion;
	}
}

bool enviar_resultado_yama(int operacion,t_serial* serial_yama) {
	t_packet yama = protocol_packet(operacion, serial_yama);
	bool result = protocol_send_packet(yama, yama_socket);
	serial_destroy(serial_yama);
	return result;
}


t_hilos* set_hilo(int etapa, char* nodo) {
	t_hilos* hilo = malloc(sizeof(t_hilos));
	hilo->etapa = etapa;
	hilo->nodo = mstring_duplicate(nodo);
	hilo->active = true;
	hilo->result = -1;
	return hilo;
}

void cargar_scripts(char* path_transf, char* path_reduc) {

	 script.fd_transf = file_open(path_transf);
	 script.script_transf = file_map(script.fd_transf);

	 script.fd_reduc = file_open(path_reduc);
	 script.script_reduc = file_map(script.fd_reduc);

}

void liberar_scripts() {

	file_unmap(script.fd_transf, script.script_transf);
	file_unmap(script.fd_reduc, script.script_reduc);
	file_close(script.fd_transf);
	file_close(script.fd_reduc);
}

const char *timeprom(mtime_t t1, mtime_t t2, int etapa){
	bool getEtapa(t_hilos* hilo){
		return (hilo->etapa == etapa);
	}
	if(mlist_count(hilos, getEtapa) == 0){
		return string_from_format("%02u:%02u:%02u:%03u", 0, 0, 0, 0);
	}else{
		mtime_t duration = abs(((int) mtime_diff(t1, t2)) / mlist_count(hilos, getEtapa));
		return mtime_formatted(duration, MTIME_DIFF | MTIME_PRECISE);
	}
}

void verificarParalelismo(int etapa){
	bool statusEtapaTransformacion(t_hilos* hilo){
		return (hilo->etapa == TRANSFORMACION && hilo->active);
	}
	bool statusEtapaReduccionLocal(t_hilos* hilo){
		return (hilo->etapa == REDUCCION_LOCAL && hilo->active);
	}
	int total;
	if (etapa == TRANSFORMACION){
		total = mlist_count(hilos, statusEtapaTransformacion);
		if (total > tareasParalelo.transf) tareasParalelo.transf = total;
	}else if (etapa == REDUCCION_LOCAL){
		total = mlist_count(hilos, statusEtapaReduccionLocal);
		if (total > tareasParalelo.reducc) tareasParalelo.reducc = total;
	}
}

void calcular_metricas(){

	bool getTransformacion(t_hilos* hilo){
		return (hilo->etapa == TRANSFORMACION);
	}
	bool getReduccionLocal(t_hilos* hilo){
		return (hilo->etapa == REDUCCION_LOCAL);
	}
	bool getReduccionGlobal(t_hilos* hilo){
		return (hilo->etapa == REDUCCION_GLOBAL);
	}

	printf("Tiempo total de ejecución del Job: %s\n",
			mtime_formatted(mtime_diff(times.job_init, times.job_end), MTIME_DIFF | MTIME_PRECISE));
	printf("Tiempo promedio de ejecución de cada etapa principal:\n"
			"Transformaciones: %s\n"
			"Reducciones Locales: %s\n"
			"Reducciones Globales: %s\n",
			timeprom(times.transf_init, times.transf_end, TRANSFORMACION),
			timeprom(times.rl_init, times.rl_end, REDUCCION_LOCAL),
			timeprom(times.rg_init, times.rg_end, REDUCCION_GLOBAL));
	printf("Cantidad total de Transformaciones realizadas en paralelo: %d\n",
			tareasParalelo.transf);
	printf("Cantidad total de Reducciones realizadas en paralelo: %d\n",
			tareasParalelo.reducc);
	printf("Cantidad total de tareas realizadas:\n"
			"Transformaciones: %d\n"
			"Reducciones Locales: %d\n"
			"Reducciones Globales: %d\n",
			mlist_count(hilos, getTransformacion),
			mlist_count(hilos, getReduccionLocal),
			mlist_count(hilos, getReduccionGlobal));
	bool getFallos(t_hilos* hilo){
		return (hilo->result == -1);
	}
	printf("Cantidad de fallos del job: %d\n", mlist_count(hilos, getFallos));
}

void init(char* argv[]){
	times.job_init = times.job_end = mtime_now();

	job.path_transf = string_duplicate(argv[1]);
	job.path_reduc = string_duplicate(argv[2]);
	job.arch = string_duplicate(argv[3]);
	job.arch_result = string_duplicate(argv[4]);
	cargar_scripts(job.path_transf, job.path_reduc);

	thread_init();
	hilos = mlist_create();
	pthread_mutex_init(&mutex_hilos, NULL);
	sem = thread_sem_create(30);
	tareasParalelo.transf = 0;
	tareasParalelo.reducc = 0;

	process_init();
	connect_to_yama();
	request_job_for_file(job.arch);
	job_active = true;
}

void terminate() {
	pthread_mutex_destroy(&mutex_hilos);
	liberar_scripts();
	socket_close(yama_socket);
	log_print("Conexión a YAMA por el socket %i cerrada", yama_socket);
	calcular_metricas();
	void hilo_destroy(t_hilos *self){
		free(self);
	}
	mlist_destroy(hilos, hilo_destroy);
}

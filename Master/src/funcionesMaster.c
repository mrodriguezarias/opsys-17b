#include "funcionesMaster.h"

void kill_thread(t_hilos* hilo){
	if(hilo->active){
		hilo->active = false;
		pthread_kill((pthread_t)hilo->hilo, -1);
	}
}

void node_drop(){
	while(job_active){
		char* data = (char*)thread_receive();
		if(data != NULL){
			bool getNodeDrop(t_hilos* hilo){
				return string_equals_ignore_case(data, hilo->nodo);
			}
			pthread_mutex_lock(&mutex_hilos);
			mlist_t* hilos_drop = mlist_find(hilos, getNodeDrop);
			mlist_traverse(hilos_drop, kill_thread);
			for(int i = 0; i < mlist_length(hilos_drop); i++){
				t_hilos* hilo_drop_node = mlist_get(hilos_drop, i);
				log_print("Finalización del hilo %d por caída del nodo: %s",
						hilo_drop_node->hilo,
						hilo_drop_node->nodo);
			}
			pthread_mutex_unlock(&mutex_hilos);
		}
	}
	thread_exit(0);
}

void actualizar_hilo(int response){
	bool getHilo(t_hilos* thread){
		return (thread->hilo == thread_self());
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
	t_packet paquete = protocol_receive_packet(socket);
	if (paquete.operation == OP_UNDEFINED) {
		*response = -1;
	} else {
		serial_unpack(paquete.content, "i", &*response);
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
	hilo->nodo = string_duplicate(nodo);
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

}

inline time_t get_current_time(){
	return time(NULL);
}

const char *datetime(time_t time){
	struct tm lt;
	localtime_r(&time, &lt);
	return string_from_format("%d-%02d-%02d %02d:%02d:%02d", lt.tm_year + 1900, lt.tm_mon + 1,
			lt.tm_mday, lt.tm_hour, lt.tm_min, lt.tm_sec);
}

const char *timediff(time_t t1, time_t t2){
	unsigned duration = abs((int) difftime(t1, t2));
	unsigned mseconds = duration % 60;
	unsigned seconds = duration % 60;
	unsigned minutes = duration / 60;
	return string_from_format("%02u:%02u:%03u", minutes, seconds, mseconds);
}

const char *timeprom(time_t t1, time_t t2, int etapa){
	bool getEtapa(t_hilos* hilo){
		return (hilo->etapa == etapa);
	}
	if(mlist_count(hilos, getEtapa) == 0){
		return string_from_format("%02u:%02u:%03u", 0, 0, 0);
	}else{
		unsigned duration = abs(((int) difftime(t1, t2)) / mlist_count(hilos, getEtapa));
		unsigned seconds = duration % 60;
		unsigned minutes = duration / 60;
		unsigned hours = duration / 60;
		return string_from_format("%02u:%02u:%03u", hours, minutes, seconds);
	}
}

void verificarParalelismo(){
	bool statusEtapaTransformacion(t_hilos* hilo){
		return (hilo->etapa == TRANSFORMACION && hilo->active);
	}
	bool statusEtapaReduccionLocal(t_hilos* hilo){
		return (hilo->etapa == REDUCCION_LOCAL && hilo->active);
	}
	int totalEtapas = mlist_count(hilos, statusEtapaTransformacion) + mlist_count(hilos, statusEtapaReduccionLocal);
	if(mlist_count(hilos, statusEtapaTransformacion) > 0 &&
			mlist_count(hilos, statusEtapaReduccionLocal) > 0 &&
			totalEtapas > tareasParalelo.total){
		tareasParalelo.total = totalEtapas;
		tareasParalelo.transf = mlist_count(hilos, statusEtapaTransformacion);
		tareasParalelo.reducc = mlist_count(hilos, statusEtapaReduccionLocal);
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

	printf("Tiempo total de ejecución del Job: %s\n", timediff(times.job_init, times.job_end));
	printf("Tiempo promedio de ejecución de cada etapa principal:\n"
			"Transformaciones: %s\n"
			"Reducciones Locales: %s\n"
			"Reducciones Globales: %s\n",
			timeprom(times.transf_init, times.transf_end, TRANSFORMACION),
			timeprom(times.rl_init, times.rl_end, REDUCCION_LOCAL),
			timeprom(times.rg_init, times.rg_end, REDUCCION_GLOBAL));
	printf("Cantidad de etapas ejecutadas en paralelo: %d\n"
			"Transformaciones: %d\n"
			"Reducciones Locales: %d\n",
			tareasParalelo.total,
			tareasParalelo.transf,
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
	times.job_init = get_current_time();

	job.path_transf = string_duplicate(argv[1]);
	job.path_reduc = string_duplicate(argv[2]);
	job.arch = string_duplicate(argv[3]);
	job.arch_result = string_duplicate(argv[4]);
	cargar_scripts(job.path_transf, job.path_reduc);

	thread_init();
	hilos = mlist_create();
	pthread_mutex_init(&mutex_hilos, NULL);

	tareasParalelo.total = 0;
	tareasParalelo.transf = 0;
	tareasParalelo.reducc = 0;

	process_init();
	connect_to_yama();
	request_job_for_file(job.arch);
	job_active = true;
}

void terminate() {
	thread_term();
	liberar_scripts();
	socket_close(yama_socket);
	log_print("Conexión a YAMA por el socket %i cerrada", yama_socket);
	calcular_metricas();
	void hilo_destroy(t_hilos *self){
		free(self);
	}
	mlist_destroy(hilos, hilo_destroy);
}

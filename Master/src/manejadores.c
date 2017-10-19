#include "manejadores.h"

void manejador_transformacion(tEtapaTransformacion* transformacion) {
	log_print("Hilo ETAPA_TRANSFORMACION");
	t_socket socket = connect_to_worker(transformacion->ip, transformacion->puerto);

	t_serial *serial_worker = serial_pack("ssii",
			script.script_transf,
			transformacion->archivo_etapa,
			transformacion->bloque,
			transformacion->bytes_ocupados);

	enviar_operacion_worker(OP_INICIAR_TRANSFORMACION, socket, serial_worker);

	int response = protocol_receive_response(socket);
	printf("Transformación %d\n", response);
	t_serial *serial_yama = serial_pack("sii",
				transformacion->nodo,
				transformacion->bloque,
				response);

	pthread_mutex_lock(&mutex_hilos);
	actualizar_hilo(response);
	pthread_mutex_unlock(&mutex_hilos);

	socket_close(socket);
	free(transformacion);
	enviar_resultado_yama(OP_TRANSFORMACION_LISTA,serial_yama);

	thread_exit(0);
}

void manejador_rl(tEtapaReduccionLocal * etapa_rl) {
	log_print("Hilo ETAPA_REDUCCION_LOCAL");
	t_socket socket = connect_to_worker(etapa_rl->ip, etapa_rl->puerto);

	t_serial *serial_worker = serial_create(NULL, 0);
	serial_add(serial_worker, "s", script.script_reduc);
	serial_add(serial_worker, "i", mlist_length(etapa_rl->archivos_temporales_de_transformacion));

	void routine(char* archivo_temp){
		serial_add(serial_worker, "s", archivo_temp);
	}
	mlist_traverse(etapa_rl->archivos_temporales_de_transformacion, routine);
	serial_add(serial_worker, "s", etapa_rl->archivo_etapa);

	enviar_operacion_worker(OP_INICIAR_REDUCCION_LOCAL, socket, serial_worker);

	int response = protocol_receive_response(socket);
	printf("Reducción local %d\n", response);

	t_serial *serial_yama = serial_pack("si", etapa_rl->nodo, response);

	pthread_mutex_lock(&mutex_hilos);
	actualizar_hilo(response);
	pthread_mutex_unlock(&mutex_hilos);

	socket_close(socket);
	free(etapa_rl);
	enviar_resultado_yama(OP_REDUCCION_LOCAL_LISTA,serial_yama);

	thread_exit(0);
}


void manejador_rg(mlist_t* list) {
	log_print("ETAPA_REDUCCION_GLOBAL");

	bool getManager(tEtapaReduccionGlobal* etapa){
		return (string_equals_ignore_case(etapa->encargado, SI));
	}
	tEtapaReduccionGlobal* worker_manager = mlist_find(list,getManager);
	t_socket socket = connect_to_worker(worker_manager->ip, worker_manager->puerto);

	t_serial *serial_worker = serial_create(NULL, 0);
	serial_add(serial_worker, "s", script.script_reduc);

	serial_add(serial_worker, "i", mlist_length(list));

	void routine(tEtapaReduccionGlobal* rg_worker){
		serial_add(serial_worker, "ssss",
				rg_worker->nodo,
				rg_worker->ip,
				rg_worker->puerto,
				rg_worker->archivo_temporal_de_rl);
	}

	mlist_traverse(list, routine);

	serial_add(serial_worker, "s", worker_manager->archivo_etapa);

	enviar_operacion_worker(OP_INICIAR_REDUCCION_GLOBAL, socket, serial_worker);

	int response = protocol_receive_response(socket);

	t_serial *serial_yama = serial_pack("si", worker_manager->nodo, response);

	actualizar_hilo(response);

	socket_close(socket);
	void etapa_rg_destroy(tEtapaReduccionGlobal *rg){
		free(rg);
	}
	mlist_destroy(list, etapa_rg_destroy);

	enviar_resultado_yama(OP_REDUCCION_GLOBAL_LISTA,serial_yama);

	thread_exit(0);
}


void manejador_af(tAlmacenadoFinal* af) {
	log_print("ETAPA_ALMACENAMIENTO_FINAL");

	t_socket socket = connect_to_worker(af->ip, af->puerto);
	t_serial *serial_worker = serial_pack("ss",af->archivo_etapa, job.arch_result);

	enviar_operacion_worker(OP_INICIAR_ALMACENAMIENTO, socket, serial_worker);

	int response = protocol_receive_response(socket);

	t_serial *serial_yama = serial_pack("si", af->nodo, response);

	actualizar_hilo(response);

	job_end = get_current_time();
	socket_close(socket);
	free(af);
	enviar_resultado_yama(OP_ALMACENAMIENTO_LISTA,serial_yama);
	job_active = false;
	thread_exit(0);
}

void etapa_transformacion(const t_packet* paquete) {
	mlist_t* listTranformacion = list_transformacion_unpack(paquete->content);

	for (int i = 0; i < mlist_length(listTranformacion); i++) {
		tEtapaTransformacion* etapa_transformacion = mlist_get(
				listTranformacion, i);
		t_hilos* hilo_transformacion = set_hilo(TRANSFORMACION);

		if ((hilo_transformacion->hilo = thread_create(manejador_transformacion,
				etapa_transformacion)) < 0) {
			log_report("Error al crear hilo en INICIAR_TRANSFORMACION");
			perror("Error al crear el hilo_transformacion");
		}

		pthread_mutex_lock(&mutex_hilos);
		mlist_append(hilos, hilo_transformacion);
		verificarParalelismo();
		pthread_mutex_unlock(&mutex_hilos);
	}
}

void etapa_replanificacion(const t_packet* paquete) {
	tEtapaTransformacion* etapa_transformacion = malloc(sizeof(tEtapaTransformacion));
	serial_unpack(paquete->content, "sssiiss",
			etapa_transformacion->nodo,
			etapa_transformacion->ip,
			etapa_transformacion->puerto,
			etapa_transformacion->bloque,
			etapa_transformacion->bytes_ocupados,
			etapa_transformacion->archivo_etapa);

	t_hilos* hilo_transformacion = set_hilo(TRANSFORMACION);

	if ((hilo_transformacion->hilo = thread_create(manejador_transformacion,
			etapa_transformacion)) < 0) {
		log_report("Error al crear hilo en INICIAR_REPLANIFICACION");
		perror("Error al crear el hilo_replanificacion");
	}

	pthread_mutex_lock(&mutex_hilos);
	mlist_append(hilos, hilo_transformacion);
	verificarParalelismo();
	pthread_mutex_unlock(&mutex_hilos);

}

void etapa_reduccion_local(const t_packet* paquete) {
	tEtapaReduccionLocal* etapa_rl = etapa_rl_unpack(paquete->content);
	t_hilos* hilo_rl = set_hilo(REDUCCION_LOCAL);

	if ((hilo_rl->hilo = thread_create(manejador_rl, etapa_rl))
			< 0) {
		log_report("Error al crear hilo en INICIAR_REDUCCION_LOCAL");
		perror("Error al crear el hilo_reduccion_local");
	}

	pthread_mutex_lock(&mutex_hilos);
	mlist_append(hilos, hilo_rl);
	verificarParalelismo();
	pthread_mutex_unlock(&mutex_hilos);
}

void etapa_reduccion_global(const t_packet* paquete) {
	mlist_t* listReduccionGlobal = list_reduccionGlobal_unpack(
			paquete->content);

	t_hilos* hilo_rg = set_hilo(REDUCCION_GLOBAL);

	if ((hilo_rg->hilo = thread_create(manejador_rg, listReduccionGlobal))
			< 0) {
		log_report("Error al crear hilo en INICIAR_REDUCCION_GLOBAL");
		perror("Error al crear el hilo_reduccion_global");
	}

	mlist_append(hilos, hilo_rg);
}

void etapa_almacenamiento(const t_packet* paquete) {
	tAlmacenadoFinal* almacenamiento = etapa_af_unpack(
			paquete->content);

	t_hilos* hilo_af = set_hilo(ALMACENAMIENTO);

	if ((hilo_af->hilo = thread_create(manejador_af, almacenamiento))
			< 0) {
		log_report("Error al crear hilo en INICIAR_ALMACENAMIENTO");
		perror("Error al crear el hilo_almacenamiento");
	}

	mlist_append(hilos, hilo_af);
	thread_wait(hilo_af->hilo);
}

void manejador_yama(t_packet paquete) {
	int exit_code;

	switch (paquete.operation) {
	case OP_INICIAR_TRANSFORMACION:
		log_print("OP_INICIAR_TRANSFORMACION");
		etapa_transformacion(&paquete);
		break;
	case OP_INICIAR_REDUCCION_LOCAL:
		log_print("OP_INICIAR_REDUCCION_LOCAL");
		etapa_reduccion_local(&paquete);
		break;
	case OP_INICIAR_REDUCCION_GLOBAL:
		log_print("OP_INICIAR_REDUCCION_GLOBAL");
		etapa_reduccion_global(&paquete);
		break;
	case OP_INICIAR_ALMACENAMIENTO:
		log_print("OP_INICIAR_ALMACENAMIENTO");
		etapa_almacenamiento(&paquete);
		break;
	case OP_INICIAR_REPLANIFICACION:
		log_print("OP_INICIAR_REPLANIFICACION");
		etapa_replanificacion(&paquete);
		break;
	case OP_ERROR_JOB:
		serial_unpack(paquete.content, "i", &exit_code);
		log_print("ERROR_JOB - Código de error: %d", exit_code);
		thread_killall();
		job_active = false;
		job_end = get_current_time();
		break;
	default:
		log_print("OP_UNDEFINED");
		break;
	}
}

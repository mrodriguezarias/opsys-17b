#include "manejadores.h"

void finalizar_manejador_transf(int response, t_socket socket,
		tEtapaTransformacion* transformacion) {
	t_serial *serial_yama = serial_pack("siis",
			transformacion->nodo,
			transformacion->bloque,
			response,
			job.arch);
	pthread_mutex_lock(&mutex_hilos);
	actualizar_hilo(response);
	pthread_mutex_unlock(&mutex_hilos);

	if(response == -1){
		log_print("Finalización del hilo %d etapa TRANSFORMACION por caída del nodo: %s",
				thread_self(),
				transformacion->nodo);
	}else{
		log_print("Finalización hilo %d TRANSFORMACION realizada", thread_self());
	}

	socket_close(socket);
	log_print("Conexión a Worker en %s:%s por el socket %i cerrada",
			transformacion->ip, transformacion->puerto, socket);
	free(transformacion);
	enviar_resultado_yama(OP_TRANSFORMACION_LISTA, serial_yama);
	log_inform("Envío a YAMA socket %d OP_TRANSFORMACION_LISTA",
			yama_socket);
	times.transf_end = get_current_time();
}

void manejador_transformacion(tEtapaTransformacion* transformacion) {
	log_print("Hilo %d creado ETAPA_TRANSFORMACION", thread_self());
	int response;

	t_socket socket = connect_to_worker(transformacion->ip, transformacion->puerto);

	if (socket == -1){
		response = -1;
		finalizar_manejador_transf(response, socket, transformacion);
		thread_send(hilo_node_drop, transformacion->nodo);
		thread_exit(0);
	}else{
		t_serial *serial_worker = serial_pack("ssii",
			script.script_transf,
			transformacion->archivo_etapa,
			transformacion->bloque,
			transformacion->bytes_ocupados);
			log_inform("Envío a %s socket %d OP_INICIAR_TRANSFORMACION",
					transformacion->nodo,
					socket);
		if(!enviar_operacion_worker(OP_INICIAR_TRANSFORMACION, socket, serial_worker)){
			response = -1;
			finalizar_manejador_transf(response, socket, transformacion);
			thread_send(hilo_node_drop, transformacion->nodo);
			thread_exit(0);
		}else{
			log_inform("Se recibe respuesta de ETAPA_TRANSFORMACION del worker en socket %d",
					socket);
			response_worker(socket, &response);
		}
	}

	finalizar_manejador_transf(response, socket, transformacion);
	thread_exit(0);
}

void finalizar_manejador_rl(int response, t_socket socket,
		tEtapaReduccionLocal* etapa_rl) {
	t_serial *serial_yama = serial_pack("si", etapa_rl->nodo, response);

	pthread_mutex_lock(&mutex_hilos);
	actualizar_hilo(response);
	pthread_mutex_unlock(&mutex_hilos);

	if(response == -1){
		log_print("Finalización del hilo %d etapa REDUCCION_LOCAL por caída del nodo: %s",
				thread_self(),
				etapa_rl->nodo);
	}else{
		log_print("Finalización hilo %d REDUCCION_LOCAL realizada", thread_self());
	}

	socket_close(socket);
	log_print("Conexión a Worker en %s:%s por el socket %i cerrada",
			etapa_rl->ip, etapa_rl->puerto, socket);
	free(etapa_rl);
	enviar_resultado_yama(OP_REDUCCION_LOCAL_LISTA, serial_yama);
	log_inform("Envío a YAMA socket %d OP_REDUCCION_LOCAL_LISTA",
			yama_socket);
	times.rl_end = get_current_time();
}

void manejador_rl(tEtapaReduccionLocal * etapa_rl) {
	log_print("Hilo %d creado ETAPA_REDUCCION_LOCAL", thread_self());
	int response;

	t_socket socket = connect_to_worker(etapa_rl->ip, etapa_rl->puerto);

	if (socket == 1){
		response = -1;
		finalizar_manejador_rl(response, socket, etapa_rl);
		thread_send(hilo_node_drop, etapa_rl->nodo);
		thread_exit(0);
	}else{
		t_serial *serial_worker = serial_create(NULL, 0);
		serial_add(serial_worker, "s", script.script_reduc);
		serial_add(serial_worker, "i", mlist_length(etapa_rl->archivos_temporales_de_transformacion));

		void routine(char* archivo_temp){
			serial_add(serial_worker, "s", archivo_temp);
		}
		mlist_traverse(etapa_rl->archivos_temporales_de_transformacion, routine);
		serial_add(serial_worker, "s", etapa_rl->archivo_etapa);

		log_inform("Envío a %s socket %d OP_INICIAR_REDUCCION_LOCAL",
				etapa_rl->nodo,
				socket);
		if(!enviar_operacion_worker(OP_INICIAR_REDUCCION_LOCAL, socket, serial_worker)){
			response = -1;
			finalizar_manejador_rl(response, socket, etapa_rl);
			thread_send(hilo_node_drop, etapa_rl->nodo);
			thread_exit(0);
		}else{
			log_inform("Se recibe respuesta de ETAPA_REDUCCION_LOCAL del worker en socket %d",
					socket);
			response_worker(socket, &response);
		}
	}

	finalizar_manejador_rl(response, socket, etapa_rl);
	thread_exit(0);
}

void finalizar_manejador_rg(int response, t_socket socket, mlist_t* list,
		tEtapaReduccionGlobal* worker) {
	t_serial *serial_yama = serial_pack("si", worker->nodo, response);

	actualizar_hilo(response);

	if(response == -1){
		log_print("Finalización del hilo %d etapa REDUCCION_GLOBAL por caída del nodo: %s",
				thread_self(),
				worker->nodo);
	}else{
		log_print("Finalización hilo %d REDUCCION_GLOBAL realizada", thread_self());
	}

	socket_close(socket);
	log_print("Conexión a Worker en %s:%s por el socket %i cerrada",
			worker->ip, worker->puerto, socket);
	void etapa_rg_destroy(tEtapaReduccionGlobal* rg) {
		free(rg);
	}
	mlist_destroy(list, etapa_rg_destroy);
	enviar_resultado_yama(OP_REDUCCION_GLOBAL_LISTA, serial_yama);
	log_inform("Envío a YAMA socket %d OP_REDUCCION_GLOBAL_LISTA",
			yama_socket);
	times.rg_end = get_current_time();
}

void manejador_rg(mlist_t* list) {
	log_print("Hilo %d creado ETAPA_REDUCCION_GLOBAL", thread_self());
	int response;

	bool getManager(tEtapaReduccionGlobal* etapa){
		return (string_equals_ignore_case(etapa->encargado, SI));
	}
	tEtapaReduccionGlobal* worker_manager = mlist_find(list,getManager);

	t_socket socket = connect_to_worker(worker_manager->ip, worker_manager->puerto);

	if (socket == 1){
		response = -1;
		finalizar_manejador_rg(response, socket, list, worker_manager);
		thread_send(hilo_node_drop, worker_manager->nodo);
		thread_exit(0);
	}else{
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

		log_inform("Envío a %s socket %d OP_INICIAR_REDUCCION_LOCAL",
				worker_manager->nodo,
				socket);
		if(!enviar_operacion_worker(OP_INICIAR_REDUCCION_GLOBAL, socket, serial_worker)){
			response = -1;
			finalizar_manejador_rg(response, socket, list, worker_manager);
			thread_send(hilo_node_drop, worker_manager->nodo);
			thread_exit(0);
		}else{
			log_inform("Se recibe respuesta de ETAPA_REDUCCION_GLOBAL del worker en socket %d",
					socket);
			response_worker(socket, &response);
		}
	}

	finalizar_manejador_rg(response, socket, list, worker_manager);
	thread_exit(0);
}

void finalizar_manejador_af(int response, t_socket socket, tAlmacenadoFinal* af) {
	t_serial *serial_yama = serial_pack("si", af->nodo, response);

	actualizar_hilo(response);

	if(response == -1){
		log_print("Finalización del hilo %d etapa ALMACENAMIENTO_FINAL por caída del nodo: %s",
				thread_self(),
				af->nodo);
	}else{
		log_print("Finalización hilo %d ALMACENAMIENTO_FINAL realizada", thread_self());
	}

	times.job_end = get_current_time();
	socket_close(socket);
	log_print("Conexión a Worker en %s:%s por el socket %i cerrada",
			af->ip, af->puerto, socket);
	free(af);
	enviar_resultado_yama(OP_ALMACENAMIENTO_LISTA, serial_yama);
	log_inform("Envío a YAMA socket %d OP_ALMACENAMIENTO_LISTA",
			yama_socket);
	job_active = false;
}

void manejador_af(tAlmacenadoFinal* af) {
	log_print("Hilo %d creado ETAPA_ALMACENAMIENTO_FINAL", thread_self());
	int response;

	t_socket socket = connect_to_worker(af->ip, af->puerto);
	if (socket == 1){
		response = -1;
		finalizar_manejador_af(response, socket, af);
		thread_send(hilo_node_drop, af->nodo);
		thread_exit(0);
	}else{
		t_serial *serial_worker = serial_pack("ss",af->archivo_etapa, job.arch_result);

		log_inform("Envío a %s socket %d OP_INICIAR_REDUCCION_LOCAL",
				af->nodo,
				socket);
		if(!enviar_operacion_worker(OP_INICIAR_ALMACENAMIENTO, socket, serial_worker)){
			response = -1;
			finalizar_manejador_af(response, socket, af);
			thread_send(hilo_node_drop, af->nodo);
			thread_exit(0);
		}else{
			log_inform("Se recibe respuesta de ETAPA_ALMACENAMIENTO_FINAL del worker en socket %d",
					socket);
			response_worker(socket, &response);
		}
	}

	finalizar_manejador_af(response, socket, af);
	thread_exit(0);
}

void etapa_transformacion(const t_packet* paquete) {
	mlist_t* listTranformacion = list_transformacion_unpack(paquete->content);

	bool getTransformacion(t_hilos* hilo){
		return (hilo->etapa == TRANSFORMACION);
	}
	if(mlist_count(hilos, getTransformacion) == 0){
		times.transf_init = get_current_time();
		times.transf_end = get_current_time();
	}

	for (int i = 0; i < mlist_length(listTranformacion); i++) {
		tEtapaTransformacion* etapa_transformacion = mlist_get(
				listTranformacion, i);
		t_hilos* hilo_transformacion = set_hilo(TRANSFORMACION, etapa_transformacion->nodo);

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

void etapa_reduccion_local(const t_packet* paquete) {
	tEtapaReduccionLocal* etapa_rl = etapa_rl_unpack(paquete->content);
	bool getReduccionLocal(t_hilos* hilo){
		return (hilo->etapa == REDUCCION_LOCAL);
	}
	if(mlist_count(hilos, getReduccionLocal) == 0){
		times.rl_init = get_current_time();
		times.rl_end = get_current_time();
	}

	t_hilos* hilo_rl = set_hilo(REDUCCION_LOCAL, etapa_rl->nodo);

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

	bool getReduccionGlobal(t_hilos* hilo){
		return (hilo->etapa == REDUCCION_GLOBAL);
	}
	if(mlist_count(hilos, getReduccionGlobal) == 0){
		times.rg_init = get_current_time();
		times.rg_end = get_current_time();
	}

	bool getManager(tEtapaReduccionGlobal* etapa){
		return (string_equals_ignore_case(etapa->encargado, SI));
	}
	tEtapaReduccionGlobal* worker_manager = mlist_find(listReduccionGlobal,getManager);

	t_hilos* hilo_rg = set_hilo(REDUCCION_GLOBAL, worker_manager->nodo);

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

	t_hilos* hilo_af = set_hilo(ALMACENAMIENTO, almacenamiento->nodo);

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
		log_inform("OP_INICIAR_TRANSFORMACION");
		etapa_transformacion(&paquete);
		break;
	case OP_INICIAR_REDUCCION_LOCAL:
		log_inform("OP_INICIAR_REDUCCION_LOCAL");
		etapa_reduccion_local(&paquete);
		break;
	case OP_INICIAR_REDUCCION_GLOBAL:
		log_inform("OP_INICIAR_REDUCCION_GLOBAL");
		etapa_reduccion_global(&paquete);
		break;
	case OP_INICIAR_ALMACENAMIENTO:
		log_inform("OP_INICIAR_ALMACENAMIENTO");
		etapa_almacenamiento(&paquete);
		break;
	case OP_ERROR_JOB:
		serial_unpack(paquete.content, "i", &exit_code);
		log_print("ERROR_JOB - Código de error: %d", exit_code);
		mlist_traverse(hilos, kill_thread);
		job_active = false;
		times.job_end = get_current_time();
		break;
	default:
		break;
	}

}

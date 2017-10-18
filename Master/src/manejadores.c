#include "manejadores.h"

void manejador_transformacion(tEtapaTransformacion* transformacion) {
	log_print("Hilo ETAPA_TRANSFORMACION");
	t_socket socket = connect_to_worker(transformacion->ip, transformacion->puerto);

	t_serial *serial_worker = serial_pack("sii",
			transformacion->archivo_etapa,
			transformacion->bloque,
			transformacion->bytes_ocupados);

	t_packet worker = protocol_packet(OP_INICIAR_TRANSFORMACION, serial_worker);
	protocol_send_packet(worker, socket);
	serial_destroy(serial_worker);

	int response = protocol_receive_response(socket);

	t_serial *serial_yama = serial_pack("sii",
				transformacion->nodo,
				transformacion->bloque,
				response);

	t_packet yama = protocol_packet(OP_INICIAR_TRANSFORMACION, serial_yama);
	protocol_send_packet(yama, yama_socket);
	serial_destroy(serial_yama);

	pthread_mutex_lock(&mutex_hilos);
	bool getHilo(t_hilos* thread){
		return (thread->hilo == thread_self());
	};
	t_hilos* hilo = mlist_find(hilos,getHilo);
	hilo->active = false;
	pthread_mutex_unlock(&mutex_hilos);

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

	t_packet worker = protocol_packet(OP_INICIAR_REDUCCION_LOCAL, serial_worker);
	protocol_send_packet(worker, socket);
	serial_destroy(serial_worker);

	int response = protocol_receive_response(socket);

	t_serial *serial_yama = serial_pack("si", etapa_rl->nodo, response);

	t_packet yama = protocol_packet(OP_INICIAR_TRANSFORMACION, serial_yama);
	protocol_send_packet(yama, yama_socket);
	serial_destroy(serial_yama);

	pthread_mutex_lock(&mutex_hilos);
	bool getHilo(t_hilos* thread){
		return (thread->hilo == thread_self());
	};
	t_hilos* hilo = mlist_find(hilos,getHilo);
	hilo->active = false;
	pthread_mutex_unlock(&mutex_hilos);

	thread_exit(0);
}

void manejador_rg(mlist_t* list) {
	log_print("Hilo ETAPA_REDUCCION_GLOBAL");
}

void etapa_transformacion(const t_packet* paquete) {
	mlist_t* listTranformacion = list_transformacion_unpack(paquete->content);
	t_hilos* hilo_transformacion = malloc(sizeof(t_hilos));
	for (int i = 0; i < mlist_length(listTranformacion); i++) {
		tEtapaTransformacion* etapa_transformacion = mlist_get(
				listTranformacion, i);
		if ((hilo_transformacion->hilo = thread_create(manejador_transformacion,
				etapa_transformacion)) < 0) {
			log_report("Error al crear hilo en INICIAR_TRANSFORMACION");
			perror("Error al crear el hilo_transformacion");
		}

		hilo_transformacion->etapa = TRANSFORMACION;
		hilo_transformacion->active = true;
		pthread_mutex_lock(&mutex_hilos);
		mlist_append(hilos, hilo_transformacion);
		verificarParalelismo();
		pthread_mutex_unlock(&mutex_hilos);
	}
}

void etapa_reduccion_local(const t_packet* paquete) {
	tEtapaReduccionLocal* etapa_rl = etapa_rl_unpack(paquete->content);
	t_hilos* hilo_rl = malloc(sizeof(t_hilos));
	if ((hilo_rl->hilo = thread_create(manejador_transformacion, etapa_rl))
			< 0) {
		log_report("Error al crear hilo en INICIAR_REDUCCION_LOCAL");
	}
	hilo_rl->etapa = REDUCCION_LOCAL;
	hilo_rl->active = true;
	pthread_mutex_lock(&mutex_hilos);
	mlist_append(hilos, hilo_rl);
	verificarParalelismo();
	pthread_mutex_unlock(&mutex_hilos);
}

void etapa_reduccion_global(const t_packet* paquete) {
	mlist_t* listReduccionGlobal = list_reduccionGlobal_unpack(
			paquete->content);
	t_hilos* hilo_rg = malloc(sizeof(t_hilos));
	for (int i = 0; i < mlist_length(listReduccionGlobal); i++) {
		tEtapaReduccionGlobal* etapa_rg = mlist_get(listReduccionGlobal, i);
		if ((hilo_rg->hilo = thread_create(manejador_transformacion, etapa_rg))
				< 0) {
			log_report("Error al crear hilo en INICIAR_TRANSFORMACION");
			perror("Error al crear el hilo_transformacion");
		}
		hilo_rg->etapa = REDUCCION_GLOBAL;
		hilo_rg->active = true;
		pthread_mutex_lock(&mutex_hilos);
		mlist_append(hilos, hilo_rg);
		pthread_mutex_unlock(&mutex_hilos);
	}
}

void manejador_yama(t_packet paquete) {

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
	default:
		break;
	}

}

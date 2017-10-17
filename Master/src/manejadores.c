#include "manejadores.h"


void manejador_transformacion(tEtapaTransformacion* transformacion) {
	log_print("Hilo ETAPA_TRANSFORMACION");
	t_socket socket = connect_to_worker(transformacion->ip, transformacion->puerto);


	t_serial *serial = serial_pack("sii",
			transformacion->archivo_etapa,
			transformacion->bloque,
			transformacion->bytes_ocupados);

	t_packet worker = protocol_packet(OP_INICIAR_TRANSFORMACION, serial);
	protocol_send_packet(worker, socket);
	//char * respuesta = socket_receive_string(master.worker_socket);

	//mandar_script(master.worker_socket,job.script_transf);
}

void manejador_rl(tEtapaReduccionLocal * etapa_rl) {
	log_print("Hilo ETAPA_REDUCCION_LOCAL");
	t_socket socket = connect_to_worker(etapa_rl->ip, etapa_rl->puerto);
}

void manejador_rg(mlist_t* list) {
	log_print("Hilo ETAPA_REDUCCION_GLOBAL");

}


void manejador_yama(t_packet paquete) {

	switch (paquete.operation) {
	case OP_INICIAR_TRANSFORMACION:
		log_print("OP_INICIAR_TRANSFORMACION");
		mlist_t* listTranformacion = list_transformacion_unpack(paquete.content);
		t_hilos* hilo_transformacion = malloc(sizeof(t_hilos));

		for(int i = 0; i < mlist_length(listTranformacion); i++){
			tEtapaTransformacion* etapa_transformacion = mlist_get(listTranformacion, i);
			if ((hilo_transformacion->hilo = thread_create(manejador_transformacion, etapa_transformacion))
					< 0) {
				log_report("Error al crear hilo en INICIAR_TRANSFORMACION");
				perror("Error al crear el hilo_transformacion");
			}

			hilo_transformacion->etapa = TRANSFORMACION;
			hilo_transformacion->active =true;
			pthread_mutex_lock(&mutex_hilos);
			mlist_append(hilos, hilo_transformacion);
			pthread_mutex_unlock(&mutex_hilos);
		}

		break;
	case OP_INICIAR_REDUCCION_LOCAL:
		log_print("OP_INICIAR_REDUCCION_LOCAL");
		tEtapaReduccionLocal* etapa_rl = etapa_rl_unpack(paquete.content);
		t_hilos* hilo_rl = malloc(sizeof(t_hilos));

		if ((hilo_rl->hilo = thread_create(manejador_transformacion, etapa_rl))
				< 0) {
			log_report("Error al crear hilo en INICIAR_REDUCCION_LOCAL");
		}
		hilo_rl->etapa = REDUCCION_LOCAL;
		hilo_rl->active =true;
		pthread_mutex_lock(&mutex_hilos);
		mlist_append(hilos, hilo_rl);
		pthread_mutex_unlock(&mutex_hilos);

		break;
	case OP_INICIAR_REDUCCION_GLOBAL:
		log_print("OP_INICIAR_REDUCCION_GLOBAL");
		mlist_t* listReduccionGlobal = list_reduccionGlobal_unpack(paquete.content);
		t_hilos* hilo_rg = malloc(sizeof(t_hilos));

		for(int i = 0; i < mlist_length(listReduccionGlobal); i++){
			tEtapaReduccionGlobal* etapa_rg = mlist_get(listReduccionGlobal, i);
			if ((hilo_rg->hilo = thread_create(manejador_transformacion, etapa_rg))
					< 0) {
				log_report("Error al crear hilo en INICIAR_TRANSFORMACION");
				perror("Error al crear el hilo_transformacion");
			}
			hilo_rg->etapa = REDUCCION_GLOBAL;
			hilo_rg->active =true;
			pthread_mutex_lock(&mutex_hilos);
			mlist_append(hilos, hilo_rg);
			pthread_mutex_unlock(&mutex_hilos);
		}

		break;
	default:
		break;
	}

}

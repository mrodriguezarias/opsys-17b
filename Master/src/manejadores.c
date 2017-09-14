#include "manejadores.h"

/*
 * INICIO
 * FORMA 1
 *
 * */
pthread_t hilo_transformacion;
pthread_t hilo_rl;
pthread_t hilo_rg;

void manejador_transformacion(tEtapaTransformacion * transformacion) {
	puts("\nManejador_transformacion");
	log_inform("Hilo ETAPA_TRANSFORMACION");
	connect_to_worker(transformacion->ip, transformacion->puerto);
	printf("Socket worker: %d \n",master.worker_socket);
	char * buffer = socket_receive_string(master.worker_socket);
	t_packet worker = protocol_receive(socket_worker);
	free(worker.content.data);
	printf("Respuesta handshake de Worker %s \n",buffer);


	t_serial serial = serial_pack("ssssii",transformacion->archivo_etapa,transformacion->nodo,transformacion->ip,transformacion->puerto,transformacion->bloque,transformacion->bytes_ocupados);
	worker = protocol_packet(INICIAR_TRANSFORMACION,serial);
	protocol_send(worker,master.worker_socket);
}

void manejador_rl(tEtapaReduccionLocal * etapa_rl) {
	log_inform("Hilo ETAPA_REDUCCION_LOCAL");
	connect_to_worker(etapa_rl->ip, etapa_rl->puerto);
}
void manejador_rg(tEtapaReduccionGlobal * etapa_rg) {
	log_inform("Hilo ETAPA_REDUCCION_GLOBAL");
	connect_to_worker(etapa_rg->ip, etapa_rg->puerto);
}

void manejador_yama(t_packet paquete) {
	tEtapaTransformacion etapa_transformacion;
	tEtapaReduccionLocal etapa_rl;
	tEtapaReduccionGlobal etapa_rg;

	switch (paquete.operation) {
	case (INICIAR_TRANSFORMACION):
		printf("INICIAR_TRANSFORMACION\n");
		serial_unpack(paquete.content, "ssssii",
				etapa_transformacion.archivo_etapa, etapa_transformacion.ip,
				etapa_transformacion.nodo, etapa_transformacion.puerto,
				&etapa_transformacion.bloque,
				&etapa_transformacion.bytes_ocupados);

		printf("archivo etapa: %s\n"
				"ip worker: %s \n"
				"puerto worker: %s \n",
				etapa_transformacion.archivo_etapa,
				etapa_transformacion.ip,
				etapa_transformacion.puerto);



		if (pthread_create(&hilo_transformacion, NULL,(void*) manejador_transformacion, &etapa_transformacion) < 0) {
			log_report("Error al crear hilo en INICIAR_TRANSFORMACION");
			perror("Error al crear el hilo_transformacion");
		}
		pthread_join(hilo_transformacion, NULL);
//			free(etapa_transformacion->archivo_etapa);
//			free(etapa_transformacion->ip);
//			free(etapa_transformacion->nodo);
//			free(etapa_transformacion->puerto);

		break;
	case (INICIAR_REDUCCION_LOCAL):
		serial_unpack(paquete.content, "ssss", etapa_rl.archivo_etapa,
				etapa_rl.ip, etapa_rl.nodo, etapa_rl.puerto);

		if (pthread_create(&hilo_rl, NULL, (void*) manejador_rl, &etapa_rl)
				< 0) {
			log_report("Error al crear hilo en INICIAR_REDUCCION_LOCAL");
		}
		pthread_join(hilo_rl, NULL);
//		free(etapa_rl->archivo_etapa);
//		free(etapa_rl->ip);
//		free(etapa_rl->nodo);
//		free(etapa_rl->puerto);
		break;
	case (INICIAR_REDUCCION_GLOBAL):
		serial_unpack(paquete.content, "sssssi", etapa_rg.archivo_etapa,
				etapa_rg.ip, etapa_rg.nodo, etapa_rg.puerto,
				etapa_rg.archivo_temporal_de_rl, &etapa_rg.encargado);

		if (pthread_create(&hilo_rg, NULL, (void*) manejador_rg, &etapa_rg)
				< 0) {
			log_report("Error al crear hilo en INICIAR_REDUCCION_GLOBAL");
		}
		pthread_join(hilo_rg, NULL);

//			free(etapa_rg->archivo_etapa);
//			free(etapa_rg->ip);
//			free(etapa_rg->nodo);
//			free(etapa_rg->puerto);
//			free(etapa_rg->archivo_temporal_de_rl);
		break;
	default:
		break;
	}

}

#include "manejadores.h"

#include <log.h>
#include <protocol.h>
#include <pthread.h>
#include <serial.h>
#include <socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <struct.h>
#include <sys/wait.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/stat.h>


#include <string.h>
#include <fcntl.h>

#include "funcionesMaster.h"
#include "Master.h"

/*
 * INICIO
 * FORMA 1
 *
 * */


void manejador_transformacion(tEtapaTransformacion* transformacion) {
	log_print("Hilo ETAPA_TRANSFORMACION");
	t_socket socket = connect_to_worker(transformacion->ip, transformacion->puerto);

	/*t_packet worker = protocol_receive_packet(socket);
	serial_destroy(worker.content);worker.operation = 0;worker.sender=PROC_MASTER;*/

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

		for(int i = 0; i < mlist_length(listTranformacion); i++){
			tEtapaTransformacion* etapa_transformacion = mlist_get(listTranformacion, i);
			if (pthread_create(&hilo_transformacion, NULL,(void*) manejador_transformacion, etapa_transformacion) < 0) {
				log_report("Error al crear hilo en INICIAR_TRANSFORMACION");
				perror("Error al crear el hilo_transformacion");
			}
			pthread_join(hilo_transformacion, NULL);
		}

		break;
	case OP_INICIAR_REDUCCION_LOCAL:
		log_print("OP_INICIAR_REDUCCION_LOCAL");
		tEtapaReduccionLocal* etapa_rl = etapa_rl_unpack(paquete.content);

		if (pthread_create(&hilo_rl, NULL, (void*) manejador_rl, etapa_rl)
				< 0) {
			log_report("Error al crear hilo en INICIAR_REDUCCION_LOCAL");
		}
		pthread_join(hilo_rl, NULL);

		break;
	case OP_INICIAR_REDUCCION_GLOBAL:
		log_print("OP_INICIAR_REDUCCION_GLOBAL");
		mlist_t* listReduccionGlobal = list_reduccionGlobal_unpack(paquete.content);

		if (pthread_create(&hilo_rg, NULL,(void*) manejador_rg, listReduccionGlobal) < 0) {
			log_report("Error al crear hilo en INICIAR_TRANSFORMACION");
			perror("Error al crear el hilo_transformacion");
		}
		pthread_join(hilo_rg, NULL);

		break;
	default:
		break;
	}

}

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


void manejador_transformacion(tEtapaTransformacionBis * transformacion) {
	log_print("Hilo ETAPA_TRANSFORMACION");
	connect_to_worker(transformacion->et.ip, transformacion->et.puerto);
//	printf("Socket worker: %d \n", master.worker_socket);
	char * buffer = socket_receive_string(master.worker_socket);
	log_print("%s",buffer);
	t_packet worker = protocol_receive(socket_worker);
	free(worker.content.data);worker.operation = 0;worker.sender=PROC_MASTER;
//	printf("Respuesta handshake de Worker: %s \n", buffer);
	t_serial serial = serial_pack("sii",
			transformacion->et.archivo_etapa,
			transformacion->et.bloque,
			transformacion->et.bytes_ocupados);

			//	printf("archivo etapa:%s\n"
//			"script:%s\n"
//			"bloque:%d\n"
//			"bytes ocupados:%d\n",transformacion->et.archivo_etapa,transformacion->srcipt_tranformador,
//			transformacion->et.bloque,
//			transformacion->et.bytes_ocupados);
	worker = protocol_packet(OP_INICIAR_TRANSFORMACION, serial);
	protocol_send(worker, master.worker_socket);
	char * respuesta = socket_receive_string(master.worker_socket);
//	printf("%s\n",respuesta);
	mandar_script(master.worker_socket,transformacion->srcipt_tranformador);
}

void manejador_rl(tEtapaReduccionLocal * etapa_rl) {
	log_print("Hilo ETAPA_REDUCCION_LOCAL");
	connect_to_worker(etapa_rl->ip, etapa_rl->puerto);
}
void manejador_rg(tEtapaReduccionGlobal * etapa_rg) {
	log_print("Hilo ETAPA_REDUCCION_GLOBAL");
	connect_to_worker(etapa_rg->ip, etapa_rg->puerto);
}



void manejador_yama(t_packet paquete,char * archivo_a_transformar,char*script_tranformador,char*script_reductor) {
	tEtapaTransformacionBis etapa_transformacion;
	tEtapaReduccionLocal etapa_rl;
	tEtapaReduccionGlobal etapa_rg;
	switch (paquete.operation) {
	case OP_INICIAR_TRANSFORMACION:
		log_print("OP_INICIAR_TRANSFORMACION");

		etapa_transformacion.et = etapa_transformacion_unpack(paquete.content);
		etapa_transformacion.archivo_trans = malloc(64);
		etapa_transformacion.srcipt_tranformador = malloc(64);
		strcpy(etapa_transformacion.archivo_trans,archivo_a_transformar);
		strcpy(etapa_transformacion.srcipt_tranformador,script_tranformador);
//		printf("archivo etapa: %s\n"
//				"ip worker: %s \n"
//				"puerto worker: %s \n"
//				"archivo_a_transformar:%s\n"
//				"script_transformador: %s \n",
//				etapa_transformacion.et.archivo_etapa,
//				etapa_transformacion.et.ip,
//				etapa_transformacion.et.puerto,
//				archivo_a_transformar,
//				script_tranformador);

		if (pthread_create(&hilo_transformacion, NULL,(void*) manejador_transformacion, &etapa_transformacion) < 0) {
			log_report("Error al crear hilo en INICIAR_TRANSFORMACION");
			perror("Error al crear el hilo_transformacion");
		}
		pthread_join(hilo_transformacion, NULL);

		break;
	case OP_INICIAR_REDUCCION_LOCAL:
		serial_unpack(paquete.content, "ssss", etapa_rl.archivo_etapa,
				etapa_rl.ip, etapa_rl.nodo, etapa_rl.puerto);

		if (pthread_create(&hilo_rl, NULL, (void*) manejador_rl, &etapa_rl)
				< 0) {
			log_report("Error al crear hilo en INICIAR_REDUCCION_LOCAL");
		}
		pthread_join(hilo_rl, NULL);
		break;
	case OP_INICIAR_REDUCCION_GLOBAL:
		serial_unpack(paquete.content, "sssssi", etapa_rg.archivo_etapa,
				etapa_rg.ip, etapa_rg.nodo, etapa_rg.puerto,
				etapa_rg.archivo_temporal_de_rl, &etapa_rg.encargado);

		if (pthread_create(&hilo_rg, NULL, (void*) manejador_rg, &etapa_rg)
				< 0) {
			log_report("Error al crear hilo en INICIAR_REDUCCION_GLOBAL");
		}
		pthread_join(hilo_rg, NULL);

		break;
	default:
		break;
	}

}

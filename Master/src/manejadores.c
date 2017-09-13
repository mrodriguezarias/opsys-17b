#include "manejadores.h"

/*
 * INICIO
 * FORMA 1
 *
 * */
pthread_t hilo_transformacion;
pthread_t hilo_rl;
pthread_t hilo_rg;

void manejador_transformacion(tEtapaTransformacion * transformacion){
	puts("Manejador_transformacion");
	connect_to_worker(transformacion->ip,transformacion->puerto);
}

void manejador_rl(tEtapaReduccionLocal * etapa_rl){
	connect_to_worker(etapa_rl->ip,etapa_rl->puerto);
}
void manejador_rg(tEtapaReduccionGlobal * etapa_rg){
	connect_to_worker(etapa_rg->ip,etapa_rg->puerto);
}

void manejador_yama(t_packet paquete) {
	tEtapaTransformacion  etapa_transformacion;
	tEtapaReduccionLocal  etapa_rl;
	tEtapaReduccionGlobal etapa_rg;


		switch (paquete.operation) {
		case (INICIAR_TRANSFORMACION):
		printf("INICIAR_TRANSFORMACION\n");
		serial_unpack(paquete.content, "ssssii",
				etapa_transformacion.archivo_etapa,
				etapa_transformacion.ip,
				etapa_transformacion.nodo,
				etapa_transformacion.puerto,
				&etapa_transformacion.bloque,
				&etapa_transformacion.bytes_ocupados);
		puts("Fin unpack\n");
		printf("archivo etapa: %s\n",etapa_transformacion.archivo_etapa);
//		printf("ip: %s\n",etapa_transformacion->ip);
//		printf("nodo: %s\n",etapa_transformacion->nodo);
//		printf("puerto: %s\n",etapa_transformacion->puerto);

			if (pthread_create(&hilo_transformacion, NULL, (void*) manejador_transformacion,&etapa_transformacion) < 0) {
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
			serial_unpack(paquete.content,"ssss",
					etapa_rl.archivo_etapa,
					etapa_rl.ip,
					etapa_rl.nodo,
					etapa_rl.puerto
					);
			if (pthread_create(&hilo_rl, NULL, (void*) manejador_rl,&etapa_rl) < 0) {
				log_report("Error al crear hilo en INICIAR_REDUCCION_LOCAL");
			}
			pthread_join(hilo_rl, NULL);
//		free(etapa_rl->archivo_etapa);
//		free(etapa_rl->ip);
//		free(etapa_rl->nodo);
//		free(etapa_rl->puerto);
			break;
		case (INICIAR_REDUCCION_GLOBAL):
			serial_unpack(paquete.content,"sssssi",
					etapa_rg.archivo_etapa,
					etapa_rg.ip,
					etapa_rg.nodo,
					etapa_rg.puerto,
					etapa_rg.archivo_temporal_de_rl,
					&etapa_rg.encargado);
			if (pthread_create(&hilo_rg, NULL, (void*) manejador_rg,&etapa_rg) < 0) {
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
//void  manejador_worker(char * etapa){
//		char * buffer;
//	switch(etapa->operation){
//	case(INICIAR_TRANSFORMACION):
//			etapa_transformacion = (tEtapaTransformacion *)etapa->content;
//			connect_to_worker(etapa_transformacion->etapa_transformacion->ip,etapa_transformacion->etapa_transformacion->puerto);
//			serial_pack(buffer,'i',etapa_transformacion);
//			break;
//	case(INICIAR_REDUCCION_LOCAL):
//			etapa_rl = (tEtapaReduccionLocal *)etapa->content;
//			break;
//	case(INICIAR_REDUCCION_GLOBAL):
//			etapa_rg = (tEtapaReduccionGlobal *)etapa->content;
//			break;
//	default:
//		break;
//	}
//}

/*
 *
 * FIN FORMA 1
 *
 *
 * */

/*
 *
 * INICIO
 * FORMA 2
 *
 * */



//void maneajdor_yama2(t_packet  paquete){
//
//			switch (paquete.operation) {
//			case (INICIAR_TRANSFORMACION):
//				if (pthread_create(&hilo_transformacion, NULL, (void*) manejador_transformacion,paquete.content) < 1) {
//					log_report(
//							"Error al crear hilo en INICIAR_TRANSFORMACION");
//				}
//				pthread_join(hilo_transformacion, NULL);
//				break;
//			case (INICIAR_REDUCCION_LOCAL):
//				if (pthread_create(&hilo_rl, NULL, (void*) manejador_rl,paquete.content) < 1) {
//					log_report(
//							"Error al crear hilo en INICIAR_REDUCCION_LOCAL");
//				}
//				pthread_join(hilo_rl, NULL);
//				break;
//			case (INICIAR_REDUCCION_GLOBAL):
//				if (pthread_create(&hilo_rg, NULL, (void*) manejador_rl,paquete.content) < 1) {
//					log_report(
//							"Error al crear hilo en INICIAR_REDUCCION_GLOBAL");
//				}
//				pthread_join(hilo_rg, NULL);
//				break;
//			default:
//				break;
//			}
//}


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
	connect_to_worker(transformacion->etapa_transformacion.ip,transformacion->etapa_transformacion.puerto);
}

void manejador_rl(tEtapaReduccionLocal * etapa_rl){
	connect_to_worker(etapa_rl->etapa_reduccion_local.ip,etapa_rl->etapa_reduccion_local.puerto);
}
void manejador_rg(tEtapaReduccionGlobal * etapa_rg){
	connect_to_worker(etapa_rg->etapa_reduccion_global.ip,etapa_rg->etapa_reduccion_global.puerto);
}

void manejador_yama(t_packet paquete) {
	tEtapaTransformacion * etapa_transformacion;
	tEtapaReduccionLocal * etapa_rl;
	tEtapaReduccionGlobal* etapa_rg;


		switch (paquete.operation) {
		case (INICIAR_TRANSFORMACION):
		serial_unpack(paquete.content, "i",
				etapa_transformacion->etapa_transformacion.archivo_etapa,
				etapa_transformacion->etapa_transformacion.ip,
				etapa_transformacion->etapa_transformacion.nodo,
				etapa_transformacion->etapa_transformacion.puerto,
				etapa_transformacion->bloque,
				etapa_transformacion->bytes_ocupados);

			if (pthread_create(hilo_transformacion, NULL, (void*) manejador_transformacion,etapa_transformacion) < 1) {
				log_report("Error al crear hilo en INICIAR_TRANSFORMACION");
			}
			pthread_join(hilo_transformacion, NULL);
			break;
		case (INICIAR_REDUCCION_LOCAL):
			serial_unpack(paquete.content,"i",
					etapa_rl->etapa_reduccion_local.archivo_etapa,
					etapa_rl->etapa_reduccion_local.ip,
					etapa_rl->etapa_reduccion_local.nodo,
					etapa_rl->etapa_reduccion_local.puerto
					);
			if (pthread_create(&hilo_rl, NULL, (void*) manejador_rl,etapa_rl) < 1) {
				log_report("Error al crear hilo en INICIAR_REDUCCION_LOCAL");
			}
			pthread_join(hilo_rl, NULL);
			break;
		case (INICIAR_REDUCCION_GLOBAL):
			serial_unpack(paquete.content,"i",
					etapa_rg->etapa_reduccion_global.archivo_etapa,
					etapa_rg->etapa_reduccion_global.ip,
					etapa_rg->etapa_reduccion_global.nodo,
					etapa_rg->etapa_reduccion_global.puerto,
					etapa_rg->archivo_temporal_de_rl,
					etapa_rg->encargado);
			if (pthread_create(&hilo_rg, NULL, (void*) manejador_rg,etapa_rg) < 1) {
				log_report("Error al crear hilo en INICIAR_REDUCCION_GLOBAL");
			}
			pthread_join(hilo_rg, NULL);
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
 * */

/*
 *
 * INICIO
 * FORMA 2
 *
 * */



void maneajdor_yama2(t_packet  paquete){

			switch (paquete.operation) {
			case (INICIAR_TRANSFORMACION):
				if (pthread_create(&hilo_transformacion, NULL, (void*) manejador_transformacion,paquete.content) < 1) {
					log_report(
							"Error al crear hilo en INICIAR_TRANSFORMACION");
				}
				pthread_join(hilo_transformacion, NULL);
				break;
			case (INICIAR_REDUCCION_LOCAL):
				if (pthread_create(&hilo_rl, NULL, (void*) manejador_rl,paquete.content) < 1) {
					log_report(
							"Error al crear hilo en INICIAR_REDUCCION_LOCAL");
				}
				pthread_join(hilo_rl, NULL);
				break;
			case (INICIAR_REDUCCION_GLOBAL):
				if (pthread_create(&hilo_rg, NULL, (void*) manejador_rl,paquete.content) < 1) {
					log_report(
							"Error al crear hilo en INICIAR_REDUCCION_GLOBAL");
				}
				pthread_join(hilo_rg, NULL);
				break;
			default:
				break;
			}
}


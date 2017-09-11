#include "manejadores.h"

/*
 * INICIO
 * FORMA 1
 *
 * */
void manejador_yama(t_packet * paquete) {

		char * buffer, etapa_serial;
		int pack_size, stat;

		switch (paquete->operation) {
		case (INICIAR_TRANSFORMACION):
			if (pthread_create(&hilo_worker, NULL, (void*) manejador_worker,paquete) < 1) {
				log_report(
						"Error al crear hilo en INICIAR_TRANSFORMACION");
			}
			pthread_join(hilo_worker, NULL);
			break;
		case (INICIAR_REDUCCION_LOCAL):
			if (pthread_create(&hilo_worker, NULL, (void*) manejador_worker,paquete) < 1) {
				log_report(
						"Error al crear hilo en INICIAR_REDUCCION_LOCAL");
			}
			pthread_join(hilo_worker, NULL);
			break;
		case (INICIAR_REDUCCION_GLOBAL):
			if (pthread_create(&hilo_worker, NULL, (void*) manejador_worker,paquete) < 1) {
				log_report(
						"Error al crear hilo en INICIAR_REDUCCION_GLOBAL");
			}
			pthread_join(hilo_worker, NULL);
			break;
		default:
			break;
		}


	}
void  manejador_worker(t_packet* etapa){
	tEtapaTransformacion * etapa_transformacion;
	tEtapaReduccionLocal * etapa_reduccion_local;
	tEtapaReduccionGlobal* etapa_reduccion_global;
	switch(){
	case()
	}
}

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

pthread_t hilo_transformacion;
pthread_t hilo_rl;
pthread_t hilo_rg;


void maneajdor_yama2(t_packet * paquete){

			switch (paquete->operation) {
			case (INICIAR_TRANSFORMACION):
				if (pthread_create(&hilo_transformacion, NULL, (void*) manejador_transformacion,paquete) < 1) {
					log_report(
							"Error al crear hilo en INICIAR_TRANSFORMACION");
				}
				pthread_join(hilo_transformacion, NULL);
				break;
			case (INICIAR_REDUCCION_LOCAL):
				if (pthread_create(&hilo_rl, NULL, (void*) manejador_rl,paquete) < 1) {
					log_report(
							"Error al crear hilo en INICIAR_REDUCCION_LOCAL");
				}
				pthread_join(hilo_rl, NULL);
				break;
			case (INICIAR_REDUCCION_GLOBAL):
				if (pthread_create(&hilo_rg, NULL, (void*) manejador_rl,paquete) < 1) {
					log_report(
							"Error al crear hilo en INICIAR_REDUCCION_GLOBAL");
				}
				pthread_join(hilo_rg, NULL);
				break;
			default:
				break;
			}
}

void manejador_transformacion(t_packet * paquete){

}

void manejador_rl(t_packet * paquete){

}
void manejador_rg(t_packet * paquete){

}

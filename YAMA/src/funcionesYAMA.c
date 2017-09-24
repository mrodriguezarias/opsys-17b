#include "funcionesYAMA.h"


void planificar(char * script){

}

void mostrar_configuracion(){
	log_inform("Muestro archivo de configuracion");
	printf("FS_IP: %s\n",				 config_get("FS_IP"));
	printf("FS_PUERTO: %s\n",			 config_get("FS_PUERTO"));
	printf("RETARDO_PLANIFICACION: %s\n",config_get("RETARDO_PLANIFICACION"));
	printf("ALGORITMO_BALANCEO: %s\n",	 config_get("ALGORITMO_BALANCEO"));
	printf("MASTER_PUERTO: %s\n",		 config_get("MASTER_PUERTO"));
}

#include "funcionesYAMA.h"
#include "YAMA.h"

static int contadorBloquesSeguidosNoAsignados = 0;
static bool asigneBloquesDeArchivo = false;

void planificar(t_workerPlanificacion planificador[], int tamaniolistaNodos, t_yfile file){
	int posicionArray;

	llenarArrayPlanificador(planificador,tamaniolistaNodos,&posicionArray);
	int bloque = 0;
	while(!asigneBloquesDeArchivo){
		if(posicionArray == tamaniolistaNodos){
			posicionArray = 0;
		}
		verificarCondicion(tamaniolistaNodos, &posicionArray,planificador, &bloque, file);
	}
}

int availabilityClock(){
	return atoi(config_get("DISP_BASE"));
}

int Disponibilidad(){
	if(strcmp("CLOCK",config_get("ALGORITMO_BALANCEO"))){
		return availabilityClock();
	}
	else{
		//return availabilityWClock();
		return 0;
	}
}

void llenarArrayPlanificador(t_workerPlanificacion planificador[],int tamaniolistaNodos,int *posicion){
	int i,MaximaDisponibilidad = 0;
	for(i=0;i<tamaniolistaNodos;i++){
		planificador[i].disponibilidad = Disponibilidad();
		if(planificador[i].disponibilidad > MaximaDisponibilidad){
			MaximaDisponibilidad = planificador[i].disponibilidad;
			*posicion = i;
		}
		t_infoNodo* nodoObtenido = mlist_get(listaNodosActivos,i);
		strcpy( planificador[i].nombreWorker, nodoObtenido->nodo);
	}
}


void verificarCondicion(int tamaniolistaNodos, int *posicion,t_workerPlanificacion planificador[],int *bloque,t_yfile file){
	t_block* infoArchivo = mlist_get(file.blocks, *bloque);
	if(planificador[*posicion].disponibilidad == 0){
		planificador[*posicion].disponibilidad = Disponibilidad();
		posicion++;
		contadorBloquesSeguidosNoAsignados++;
	}else if(strcmp(infoArchivo->copies[0].node, planificador[*posicion].nombreWorker) ||  strcmp(infoArchivo->copies[1].node, planificador[*posicion].nombreWorker)){
		planificador[*posicion].disponibilidad--;
		mlist_append(planificador[*posicion].bloque, bloque);
		if(mlist_length(file.blocks) == *bloque){
			asigneBloquesDeArchivo = true;
		}
		*bloque = *bloque + 1;
		*posicion = *posicion +1;
		contadorBloquesSeguidosNoAsignados = 0;
	}
	else{
		posicion++;
		contadorBloquesSeguidosNoAsignados++;
		if(contadorBloquesSeguidosNoAsignados == tamaniolistaNodos){
			int i;
			for(i = 0; i < tamaniolistaNodos; i++){
				planificador[i].disponibilidad += Disponibilidad();
			}
			contadorBloquesSeguidosNoAsignados = 0;
		}
	}

}



respuestaOperacion* serial_unpackRespuestaOperacion(t_serial *serial){
	respuestaOperacion* operacion = malloc(sizeof(respuestaOperacion));
		serial_unpack(serial, "sii", &operacion->nodo, &operacion->bloque,
				&operacion->response);
		return operacion;
}

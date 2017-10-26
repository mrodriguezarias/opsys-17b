#include "funcionesYAMA.h"
#include "YAMA.h"

static int contadorBloquesSeguidosNoAsignados = 0;
static bool asigneBloquesDeArchivo = false;

void planificar(t_workerPlanificacion planificador[], int tamaniolistaNodos, mlist_t* listaBloque){
	int posicionArray;

	llenarArrayPlanificador(planificador,tamaniolistaNodos,&posicionArray);
	int bloque = 0;
	while(!asigneBloquesDeArchivo){
		if(posicionArray == tamaniolistaNodos){
			posicionArray = 0;
		}
		verificarCondicion(tamaniolistaNodos, &posicionArray,planificador, &bloque, listaBloque);
	}
}

int availabilityClock(){
	return atoi(config_get("DISP_BASE"));
}

int Disponibilidad(){
	if(!strcmp("CLOCK",config_get("ALGORITMO_BALANCEO"))){
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
		planificador[i].nombreWorker = malloc(sizeof(char)*6);
		strcpy( planificador[i].nombreWorker, nodoObtenido->nodo);
		planificador[i].bloque = mlist_create();

	}
}


void verificarCondicion(int tamaniolistaNodos, int *posicion,t_workerPlanificacion planificador[],int *bloque,mlist_t* listaBloque){
	void* infoArchivoObtenido = mlist_get(listaBloque, *bloque);
	t_block* infoArchivo = (t_block*) infoArchivoObtenido;
	if(planificador[*posicion].disponibilidad == 0){
		planificador[*posicion].disponibilidad = Disponibilidad();
		posicion++;
		contadorBloquesSeguidosNoAsignados++;
	}else if(!strcmp(infoArchivo->copies[0].node, planificador[*posicion].nombreWorker) || !strcmp(infoArchivo->copies[1].node, planificador[*posicion].nombreWorker)){
		planificador[*posicion].disponibilidad--;
		mlist_append(planificador[*posicion].bloque,(void*)*bloque);
		*bloque = *bloque + 1;
		if(mlist_length(listaBloque) == *bloque){
			asigneBloquesDeArchivo = true;
		}
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

#include "funcionesYAMA.h"

static int contadorBloquesSeguidosNoAsignados = 0;
static bool asigneBloquesDeArchivo = false;

void planificar(t_workerPlanificacion * planificador[], int tamaniolistaNodos){
	int posicionArray;

	llenarArrayPlanificador(planificador,tamaniolistaNodos,&posicionArray);
	int bloque = 0;
	while(!asigneBloquesDeArchivo){
		if(posicionArray == tamaniolistaNodos){
			posicionArray = 0;
		}
		verificarCondicion(tamaniolistaNodos, &posicionArray,planificador, &bloque);
	}
}

int availabilityClock(){
	return config_get("DISP_BASE");
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
			posicion = i;
		}
		t_infoNodo* nodoObtenido = mlist_get(listaNodosConectados,i);
		strcpy( planificador[i].nombreWorker, nodoObtenido->nodo);
	}
}


void verificarCondicion(int tamaniolistaNodos, int *posicion,t_workerPlanificacion planificador[],int *bloque){
	tinformacionArchivo infoArchivo = mlist_get(listInformacionArch, bloque);
	if(planificador[posicion].disponibilidad == 0){
		planificador[posicion].disponibilidad = Disponibilidad();
		posicion++;
		contadorBloquesSeguidosNoAsignados++;
	}else if(strcmp(infoArchivo.copies[0].node, planificador[posicion]->nombreWorker) ||  strcmp(infoArchivo.copies[1].node, planificador[posicion]->nombreWorker)){
		planificador[posicion].disponibilidad--;
		list_append(planificador[posicion].bloque, bloque);
		if(mlist_length(listInformacionArch) == bloque){
			asigneBloquesDeArchivo = true;
		}
		bloque++;
		posicion++;
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

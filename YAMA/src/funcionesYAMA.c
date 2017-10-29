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
		usleep(retardoPlanificacion);
		verificarCondicion(tamaniolistaNodos, &posicionArray,planificador, &bloque, listaBloque);
	}
}

int availabilityClock(){
	return atoi(config_get("DISP_BASE"));
}

int Disponibilidad(){
	//string_equals_ignore_case("CLOCK",algoritmoBalanceo)
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



respuestaOperacionTranf* serial_unpackRespuestaOperacion(t_serial *serial){
	respuestaOperacionTranf* operacion = malloc(sizeof(respuestaOperacionTranf));
		serial_unpack(serial, "isiis",&operacion->idJOB ,&operacion->nodo, &operacion->bloque,
				&operacion->response,&operacion->file);
		return operacion;
}

respuestaOperacion* serial_unpackrespuestaOperacion(t_serial * serial){
	respuestaOperacion* operacion = malloc(sizeof(respuestaOperacion));
	serial_unpack(serial,"isi",&operacion->idJOB,&operacion->nodo,&operacion->response);
	return operacion;
}

t_pedidoTrans* serial_unpackPedido(t_serial* serial){
	t_pedidoTrans* operacion = malloc(sizeof(t_pedidoTrans));
	serial_unpack(serial,"is",&operacion->idJOB,&operacion->file);
	return operacion;
}

void requerirInformacionFilesystem(t_serial *file){
	t_packet packetInfoFs = protocol_packet(OP_REQUEST_FILE_INFO, file);
	protocol_send_packet(packetInfoFs,yama.fs_socket);
	serial_destroy(packetInfoFs.content);
}

void abortarJob(int socketMaster, int codigoError){
	t_packet packetError = protocol_packet(OP_ERROR_JOB, serial_pack("i",codigoError));
	protocol_send_packet(packetError, socketMaster);
	serial_destroy(packetError.content);
}

int reciboInformacionSolicitada(mlist_t* listaNodosActivos,t_yfile* Datosfile,int master){ //listaNodosactivos y datos file luego borrarlo, no lo debe recibir, es para el hardcodeo
	t_packet packetNodosActivos = protocol_receive_packet(yama.fs_socket);
	if(packetNodosActivos.operation == OP_NODES_ACTIVE_INFO){
	listaNodosActivos = nodelist_unpack(packetNodosActivos.content);
	}
		t_packet packetArchivo = protocol_receive_packet(yama.fs_socket);
		if(packetArchivo.operation == OP_ARCHIVO_INEXISTENTE){
			abortarJob(master,ARCHIVO_INEXISTENTE);
			return -1;
		}

		else if(packetArchivo.operation == OP_ARCHIVO_NODES){
			Datosfile = yfile_unpack(packetArchivo.content);
			return 0;
		}
		return 0;
}

char* generarNombreArchivoTemporalTransf(int job,int master, int bloque){
	char* nombreArchivoTemporal = malloc(sizeof(char)*21);
 sprintf(nombreArchivoTemporal,"/tmp/j%dMaster%d-temp%d",job,master,bloque);
 return nombreArchivoTemporal;
}

void agregarAtablaEstado(int job, char* nodo,int Master,int bloque,char* etapa,char* archivo_temporal,char* estado){
	t_Estado*  nuevoEstado = malloc(sizeof(t_Estado));
	nuevoEstado->job = job;
	nuevoEstado->master = Master;
	nuevoEstado->nodo = nodo;
	nuevoEstado->block = bloque;
	nuevoEstado->etapa = etapa;
	nuevoEstado->archivoTemporal = archivo_temporal;
	nuevoEstado->estado = estado;
	mlist_append(listaEstados,nuevoEstado);

}

void actualizoTablaEstado(char* nodo,int bloque,int socketMaster,int job,char* estado){
	t_Estado* estadoActual;
	t_Estado* nuevoEstado = malloc(sizeof(t_Estado));
	bool condition(void* estadoTarea){
	  	return ((t_Estado *) estadoTarea)->nodo == nodo && ((t_Estado *) estadoTarea)->block == bloque && ((t_Estado *) estadoTarea)->master == socketMaster && ((t_Estado *) estadoTarea)->job == job ? true : false;
	}
	int posicion = mlist_index(listaEstados,(void*) condition);
	void* estadoActualObtenido = mlist_get(listaEstados,posicion);
	estadoActual = (t_Estado*) estadoActualObtenido;
	nuevoEstado->job = estadoActual->job;
	nuevoEstado->master = estadoActual->master;
	nuevoEstado->nodo = estadoActual->nodo;
	nuevoEstado->block = estadoActual->block;
	nuevoEstado->etapa = estadoActual->etapa;
	nuevoEstado->archivoTemporal = estadoActual->archivoTemporal;
	nuevoEstado->estado = estado;
	mlist_replace(listaEstados,posicion,nuevoEstado);
}

void destruirlista(void* bloqueobtenido) {
		free(bloqueobtenido);
}


void replanifacion(char* nodo, const char* pathArchivo,int master,int job){
	bool aborto = false;
	t_serial* serial_send = serial_pack("s",pathArchivo);
	t_yfile* Datosfile = malloc(sizeof(t_yfile));
	requerirInformacionFilesystem(serial_send);
	//reciboInformacionSolicitada(master);
	Datosfile->blocks = mlist_create();

	bool esNodoBuscado(void* estadoTarea){
		return string_equals_ignore_case(((t_Estado *) estadoTarea)->nodo,nodo) && ((t_Estado *) estadoTarea)->master == master && ((t_Estado *) estadoTarea)->job == job && string_equals_ignore_case(((t_Estado *) estadoTarea)->etapa,"Transformacion");
	}

	mlist_t* listaFiltradaEstadosBloquesDelNodo = mlist_filter(listaEstados, (void*)esNodoBuscado);

	int i;

	mlist_t * listaDeBloquesAReplanificar = mlist_create();

	for(i=0; i < mlist_length(listaFiltradaEstadosBloquesDelNodo); i++){
		void* estadoActualBloqueObtenido = mlist_get(listaFiltradaEstadosBloquesDelNodo, i);
		t_Estado *  estadoActualBloque = (t_Estado*) estadoActualBloqueObtenido;
		actualizoTablaEstado(estadoActualBloque->nodo,estadoActualBloque->block,master,job,"Error");
		bool esBloqueBuscado(void* bloqueActual){
		  	return ((t_block *) bloqueActual)->copies[0].blockno == estadoActualBloque->block || ((t_block *) bloqueActual)->copies[1].blockno == estadoActualBloque->block;
		}
		void* bloqueDeListaObtenido = mlist_find(Datosfile->blocks, (void*)esBloqueBuscado);
		t_block * bloqueDeLista = (t_block *) bloqueDeListaObtenido;
		mlist_append(listaDeBloquesAReplanificar, bloqueDeLista);
	}

	mlist_destroy(Datosfile->blocks, destruirlista);

	mlist_t* list_to_send = mlist_create();


	for(i=0; i < mlist_length(listaDeBloquesAReplanificar); i++){
		usleep(retardoPlanificacion);
		void* bloqueDeListaObtenido = mlist_get(listaDeBloquesAReplanificar, i);
		t_block* bloqueAReplanificar = (t_block *) bloqueDeListaObtenido;
		if(string_equals_ignore_case(nodo, bloqueAReplanificar->copies[0].node) && NodoConCopia_is_active(bloqueAReplanificar->copies[1].node) ){
			bool condicion(void* datosDeUnNodo){
				return string_equals_ignore_case(((t_infoNodo *) datosDeUnNodo)->nodo , bloqueAReplanificar->copies[1].node) ? true : false;
			}
			t_infoNodo* datosNodoAEnviar = mlist_find(listaNodosActivos, condicion);
			char* nombreArchivoTemporal = malloc(sizeof(char)*21);
			strcpy(nombreArchivoTemporal, generarNombreArchivoTemporalTransf(job, master, bloqueAReplanificar->copies[1].blockno));
			tEtapaTransformacion* et = new_etapa_transformacion(bloqueAReplanificar->copies[1].node,datosNodoAEnviar->ip,datosNodoAEnviar->puerto,bloqueAReplanificar->copies[1].blockno,bloqueAReplanificar->size,nombreArchivoTemporal);
			mlist_append(list_to_send,et);
		}
		else if(string_equals_ignore_case(nodo, bloqueAReplanificar->copies[1].node) && NodoConCopia_is_active(bloqueAReplanificar->copies[0].node)){
			bool condicion(void* datosDeUnNodo){
				return string_equals_ignore_case(((t_infoNodo *) datosDeUnNodo)->nodo , bloqueAReplanificar->copies[0].node);
			}
			t_infoNodo* datosNodoAEnviar = mlist_find(listaNodosActivos, condicion);
			char* nombreArchivoTemporal = malloc(sizeof(char)*21);
			strcpy(nombreArchivoTemporal, generarNombreArchivoTemporalTransf(job,master,bloqueAReplanificar->copies[0].blockno));
			tEtapaTransformacion* et = new_etapa_transformacion(bloqueAReplanificar->copies[0].node,datosNodoAEnviar->ip,datosNodoAEnviar->puerto,bloqueAReplanificar->copies[0].blockno,bloqueAReplanificar->size,nombreArchivoTemporal);
			mlist_append(list_to_send,et);
		}
		else{
			abortarJob(master,ERROR_REPLANIFICACION);
			aborto = true;
		}
	}
	if(!aborto){
		for(i=0; i< mlist_length(list_to_send);i++){
			void* etapaObtenida = mlist_get(list_to_send,i);
			tEtapaTransformacion* etapa = (tEtapaTransformacion*) etapaObtenida;
			agregarAtablaEstado(job,etapa->nodo,master,etapa->bloque,"Transformacion",etapa->archivo_etapa,"En proceso");
		}
		mandar_etapa_transformacion(list_to_send,master);
	}
	free(Datosfile);
	mlist_destroy(listaFiltradaEstadosBloquesDelNodo,destruirlista);
	mlist_destroy(listaDeBloquesAReplanificar,destruirlista);
	mlist_destroy(list_to_send,destruirlista);
}


bool NodoConCopia_is_active(char* nodo){
	bool contieneNodo(void* unNodo){
		return string_equals_ignore_case(((t_infoNodo*) nodo)->nodo, nodo);
	}

	return mlist_any(listaNodosActivos, (void*) contieneNodo);

}

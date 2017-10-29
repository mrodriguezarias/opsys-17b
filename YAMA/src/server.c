#include "server.h"

#include <config.h>
#include <log.h>
#include <mstring.h>
#include <process.h>
#include <protocol.h>
#include <serial.h>
#include <socket.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <struct.h>
#include "funcionesYAMA.h"
#include "YAMA.h"



void listen_to_master() {
	log_print("Escuchando puertos de master");
	t_socket sv_sock = socket_init(NULL, config_get("MASTER_PUERTO"));
	t_fdset sockets = socket_set_create();
	socket_set_add(sv_sock, &sockets);
	while(true) {
		t_fdset selected = socket_select(sockets);

		for(t_socket sock = 3; sock <= sockets.max; sock++) {
			if(!socket_set_contains(sock, &selected)) continue;
			if(sock == sv_sock) {
				t_socket cli_sock = socket_accept(sv_sock);
				t_packet packet = protocol_receive_packet(cli_sock);
				if(packet.operation == OP_HANDSHAKE && packet.sender == PROC_MASTER) {
					socket_set_add(cli_sock, &sockets);
					log_inform("Conectado proceso Master por socket %i", cli_sock);
					log_inform("IdCliente otorgado: %d ",numeroJob);
					t_packet packetID = protocol_packet(OP_IDJOB, serial_pack("i",numeroJob));
					protocol_send_packet(packetID,cli_sock);
					numeroJob++;
				} else {
					socket_close(cli_sock);
				}
				serial_destroy(packet.content);
			} else {
				t_packet packetOperacion = protocol_receive_packet(sock);
				if(packetOperacion.operation == OP_UNDEFINED) {
					socket_close(sock);
					socket_set_remove(sock, &sockets);
					continue;
				}

				switch(packetOperacion.operation) {
				case OP_INIT_JOB:
					{
					t_yfile* Datosfile = malloc(sizeof(t_yfile));
					listaNodosActivos = mlist_create(); //luego borrar es para el hardcodeo
					Datosfile->blocks = mlist_create(); //luego borrar es para el hardcodeo
					log_print("OP_INIT_JOB");
					//requerirInformacionFilesystem(packetOperacion.content);
					//if(reciboInformacionSolicitada(listaNodosActivos,Datosfile,sock)==0){
					///////////////////////////// hardcodeado
					t_infoNodo* UnNodo = malloc(sizeof(t_infoNodo));
					t_infoNodo* UnNodo2 = malloc(sizeof(t_infoNodo));

					UnNodo->nodo = "NODO1";
					UnNodo->ip = "127.0.0.1";
					UnNodo->puerto = "9262";
					UnNodo2->nodo = "NODO2";
					UnNodo2->ip = "127.0.0.1";
					UnNodo2->puerto = "9263";

					mlist_append(listaNodosActivos,UnNodo);
					mlist_append(listaNodosActivos,UnNodo2);

					t_block* bloque = malloc(sizeof(t_block));
					bloque->index = 0;
					bloque->size = 10180;
					bloque->copies[0].blockno = 8;
					bloque->copies[0].node = "NODO1";
					bloque->copies[1].blockno = 11;
					bloque->copies[1].node = "NODO2";
					mlist_append(Datosfile->blocks,bloque);


					/////////////////////////////////
					int tamaniolistaNodos = mlist_length(listaNodosActivos);
					t_workerPlanificacion planificador[tamaniolistaNodos];
					planificar(planificador, tamaniolistaNodos,Datosfile->blocks);
					enviarEtapa_transformacion_Master(tamaniolistaNodos,planificador,listaNodosActivos,Datosfile->blocks,sock);
					//}
					}
					break;
				case OP_TRANSFORMACION_LISTA :
					{respuestaOperacionTranf* finalizoOperacion = serial_unpackRespuestaOperacion(packetOperacion.content);
					if(finalizoOperacion->response == -1){
						replanifacion(finalizoOperacion->nodo,finalizoOperacion->file,sock,finalizoOperacion->idJOB);
						//actualizoTablaEstado(finalizoOperacion.nodo,finalizoOperacion.bloque,"Transformacion");hay que agregar una entrada para el nuevo nodo que va a ejecutar
					}
					else{
						actualizoTablaEstado(finalizoOperacion->nodo,finalizoOperacion->bloque,sock,finalizoOperacion->idJOB,"FinalizadoOK");
						if(verificoFinalizacionTransformacion(finalizoOperacion->nodo,sock,finalizoOperacion->idJOB)){
							t_infoNodo* IP_PUERTOnodo = BuscoIP_PUERTO(finalizoOperacion->nodo);
							mlist_t* archivosTemporales_Transf = BuscoArchivosTemporales(finalizoOperacion->nodo,sock,finalizoOperacion->idJOB);
							char* temporal_local = generarNombreTemporal_local(finalizoOperacion->nodo,sock,finalizoOperacion->idJOB);
							mandarEtapaReduccionLocal(finalizoOperacion->idJOB,sock,finalizoOperacion->nodo,IP_PUERTOnodo,archivosTemporales_Transf,temporal_local);
						}
					}
					}
				break;
				case OP_REDUCCION_LOCAL_LISTA:
					{respuestaOperacion* finalizoRL = serial_unpackrespuestaOperacion(packetOperacion.content);
					if(finalizoRL->response == -1){

						actualizoTablaEstado(finalizoRL->nodo,-1,sock,finalizoRL->idJOB,"Error");
						abortarJob(sock,ERROR_REDUCCION_LOCAL);
					}
					else{
						actualizoTablaEstado(finalizoRL->nodo,-1,sock,finalizoRL->idJOB,"FinalizadoOK");
						if(verificoFinalizacionRl(finalizoRL->idJOB,sock)){
							mandarEtapaReduccionGL(sock,finalizoRL->idJOB);
						}
					}
					}
				break;
				case OP_REDUCCION_GLOBAL_LISTA:
					{respuestaOperacion* finalizoRG = serial_unpackrespuestaOperacion(packetOperacion.content);
					if(finalizoRG->response == -1){
						actualizoTablaEstado(finalizoRG->nodo,-2,sock,finalizoRG->idJOB,"Error");
						abortarJob(sock,ERROR_REDUCCION_GLOBAL);
					}
					else{
						actualizoTablaEstado(finalizoRG->nodo,-2,sock,finalizoRG->idJOB,"FinalizadoOK");
						mandarEtapaAlmacenadoFinal(finalizoRG->nodo,sock,finalizoRG->idJOB);

					}
					}
				break;
				case OP_ALMACENAMIENTO_LISTA:
					{respuestaOperacion* finalizoAF = serial_unpackrespuestaOperacion(packetOperacion.content);
					if(finalizoAF->response == -1){
						actualizoTablaEstado(finalizoAF->nodo,-3,sock,finalizoAF->idJOB,"Error");
					}
					else{
						actualizoTablaEstado(finalizoAF->nodo,-3,sock,finalizoAF->idJOB,"FinalizadoOK");
					}
					}
				break;
				default:
					log_report("Operación desconocida: %s", packetOperacion.operation);
				break;

				}
			}
		}
	}
}

void enviarEtapa_transformacion_Master(int tamaniolistaNodos,t_workerPlanificacion planificador[],mlist_t* listaNodosActivos,mlist_t* listabloques,int sock){
	mlist_t* lista = mlist_create();
	int i, j = 0;
	int nroIndex;
	for(i = 0; i < tamaniolistaNodos; i++){
		for(j = 0; j < mlist_length(planificador[i].bloque); j++){
			void* nroIndexOBtenido = mlist_get(planificador[i].bloque, j);
			nroIndex = (int)nroIndexOBtenido;

		  t_infoNodo* datosNodoAEnviar = mlist_get(listaNodosActivos, i);

		  bool condition(void* datosDeUnBloque){
		  	return ((t_block *) datosDeUnBloque)->index == nroIndex ? true : false;
		  }
		  void* datosDeUnBloqueObtenido = mlist_find(listabloques,(void*) condition);
		  t_block* datosDeUnBloque = (t_block*)datosDeUnBloqueObtenido;
		  int nroBloque;
		  if(!strcmp(datosDeUnBloque->copies[0].node, datosNodoAEnviar->nodo)){
			  nroBloque = datosDeUnBloque->copies[0].blockno;
		  } else{
			  nroBloque = datosDeUnBloque->copies[1].blockno;
		  }
		  char* nombreArchivoTemporal = malloc(sizeof(char)*21);
		  strcpy(nombreArchivoTemporal, generarNombreArchivoTemporalTransf(numeroJob,sock, nroBloque));
		  tEtapaTransformacion* et = new_etapa_transformacion(datosNodoAEnviar->nodo,datosNodoAEnviar->ip,datosNodoAEnviar->puerto,nroBloque, datosDeUnBloque->size,nombreArchivoTemporal); //ROMPE EN ESTA funcion
		  mlist_append(lista,et);
		  agregarAtablaEstado(numeroJob,datosNodoAEnviar->nodo,sock,nroBloque,"Transformacion",nombreArchivoTemporal,"En proceso");
		}
	}
	mandar_etapa_transformacion(lista,sock);

}

bool verificoFinalizacionTransformacion(char* nodo,int socket,int job){
	bool esNodoBuscado(void* estadoTarea){
		  	return string_equals_ignore_case(((t_Estado *) estadoTarea)->nodo,nodo) && ((t_Estado *) estadoTarea)->master == socket && !string_equals_ignore_case(((t_Estado *) estadoTarea)->estado, "Error") && ((t_Estado *) estadoTarea)->job == job;
	}

	mlist_t* listaFiltradaDelNodo = mlist_filter(listaEstados, (void*)esNodoBuscado);

	bool FinalizacionDeTransf_Nodo(void* estadoTarea){
			  	return string_equals_ignore_case(((t_Estado *) estadoTarea)->estado,"TerminadoOK");
	}


	return mlist_all(listaFiltradaDelNodo, (void*) FinalizacionDeTransf_Nodo);

}

void mandarEtapaReduccionLocal(int job, int socket,char* nodo,t_infoNodo* nodo_worker,mlist_t* archivos_transf,char* archivoTemporal_local){
	tEtapaReduccionLocal* etapaRL = new_etapa_rl(nodo,nodo_worker->ip,nodo_worker->puerto,archivos_transf,archivoTemporal_local);
	agregarAtablaEstado(job,nodo,socket,-1,"ReduccionLocal",archivoTemporal_local,"En proceso");
	mandar_etapa_rl(etapaRL,socket);

}

t_infoNodo* BuscoIP_PUERTO(char* nodo){
	t_infoNodo* Nodo;
	bool esNodoBuscado(void* unNodoConectado){
			  	return !strcmp(((t_Estado *) unNodoConectado)->nodo,nodo);
	}

	 void* NodoObtenido = mlist_find(listaNodosActivos, (void*) esNodoBuscado);
	 Nodo= (t_infoNodo*) NodoObtenido;
	 return Nodo;
}

mlist_t* BuscoArchivosTemporales(char* nodo,int socket,int job){

	bool esNodoBuscado(void* estadoTarea){
		  	return string_equals_ignore_case(((t_Estado *) estadoTarea)->nodo,nodo) && ((t_Estado *) estadoTarea)->master == socket && ((t_Estado *) estadoTarea)->job == job && string_equals_ignore_case(((t_Estado *) estadoTarea)->estado, "FinalizadoOK");
	}

	mlist_t* listaFiltradaDelNodo = mlist_filter(listaEstados, (void*)esNodoBuscado);

	 char* getArchivoTemporal(void* unEstadoObtenido){
		t_Estado* Estado;
		Estado = (t_Estado*) unEstadoObtenido;
		return Estado->archivoTemporal;
	}

	mlist_t* archivosTemporales = mlist_map(listaFiltradaDelNodo, (void*) getArchivoTemporal);
	return archivosTemporales;
}


char* generarNombreTemporal_local(char* nodo,int master,int job){
	char* nombreArchivoTemporal = malloc(sizeof(char)*21);
	sprintf(nombreArchivoTemporal,"/tmp/J%dMaster%d-%s",job,master,nodo);
	return nombreArchivoTemporal;

}




bool verificoFinalizacionRl(int job, int master){
	bool esJobBuscado(void* estadoTarea){
			  	return ((t_Estado *) estadoTarea)->master == master && ((t_Estado *) estadoTarea)->job == job && string_equals_ignore_case(((t_Estado *) estadoTarea)->etapa, "ReduccionLocal");
		}

	mlist_t* listaFiltradaDelNodo = mlist_filter(listaEstados, (void*)esJobBuscado);

	bool FinalizacionDeRl_Job(void* estadoTarea){
				  	return string_equals_ignore_case(((t_Estado *) estadoTarea)->estado,"TerminadoOK");
		}


		return mlist_all(listaFiltradaDelNodo, (void*) FinalizacionDeRl_Job);

}

void mandarEtapaReduccionGL(int master,int job){
	mlist_t* listaRG = mlist_create();
	mlist_t* NodosRL = BuscoNodos(master,job);
	char* nodo = seleccionarEncargado(NodosRL);
	char* nombreRG = generarArchivoRG(master,job);
	int i;
	for(i=0; i< mlist_length(NodosRL); i++){
	void* unNodo_sendObtenido	= mlist_get(NodosRL,i);
	t_nodotemporal* unNodo_send = (t_nodotemporal*) unNodo_sendObtenido;
	t_infoNodo* infoNodo = BuscoIP_PUERTO(unNodo_send->nodo);
	if(string_equals_ignore_case(unNodo_send->nodo,nodo)){
	tEtapaReduccionGlobal* etapaRG = new_etapa_rg(unNodo_send->nodo,infoNodo->ip,infoNodo->puerto,unNodo_send->archivoTemporal,nombreRG,"SI");
	mlist_append(listaRG,etapaRG);
	}
	else{
		tEtapaReduccionGlobal* etapaRG = new_etapa_rg(unNodo_send->nodo,infoNodo->ip,infoNodo->puerto,unNodo_send->archivoTemporal,"-1","NO");
		mlist_append(listaRG,etapaRG);
	}
	}
	agregarAtablaEstado(job,nodo,master,-2,"ReduccionGlobal",nombreRG,"En proceso");
	mandar_etapa_rg(listaRG,master);

}

mlist_t* BuscoNodos(int master, int job){

	bool NodosReadyRL(void* unEstadoObtenido){
		t_Estado* Estado = (t_Estado*) unEstadoObtenido;
		return Estado->master == master && Estado->job == job && string_equals_ignore_case(Estado->etapa, "ReduccionLocal") ? true : false;
	}

	mlist_t* listaFiltrada = mlist_filter(listaEstados,(void*) NodosReadyRL);

	t_nodotemporal* getNodoTemporal(void* unEstadoObtenido){
			t_Estado* Estado;
			t_nodotemporal* info_enviar_toRG = malloc(sizeof(t_nodotemporal));
			Estado = (t_Estado*) unEstadoObtenido;
			info_enviar_toRG->nodo = Estado->nodo;
			info_enviar_toRG->archivoTemporal = Estado->archivoTemporal;
			return info_enviar_toRG;
		}

	mlist_t* Nodos = mlist_map(listaFiltrada, (void*) getNodoTemporal);
	return Nodos;
}


char* seleccionarEncargado(mlist_t* NodosRL){
	return "NODO1"; //hardcodeado por el momento

}


char* generarArchivoRG(int master, int job){
	char* nombreArchivoTemporal = malloc(sizeof(char)*21);
	sprintf(nombreArchivoTemporal,"/tmp/J%dMaster%d-final",job,master);
	return nombreArchivoTemporal;

}

void mandarEtapaAlmacenadoFinal(char* nodo,int master,int idJOB){
	t_infoNodo* nodo_send = BuscoIP_PUERTO(nodo);
	char* archivo_AF = BuscoNodoEncargado(idJOB);
	tAlmacenadoFinal* af = new_etapa_af(nodo,nodo_send->ip,nodo_send->puerto,archivo_AF);
	agregarAtablaEstado(idJOB,nodo,master,-3,"AlmacenamientoFinal",archivo_AF,"En proceso");
	mandar_etapa_af(af,master);

}

char* BuscoNodoEncargado(int job){

	bool esEncargado(void* unEstadoObtenido){
			t_Estado* Estado = (t_Estado*) unEstadoObtenido;
			return Estado->job == job && string_equals_ignore_case(Estado->etapa, "ReduccionGlobal") ? true : false;
	}


	void* nodoEncargadoObtenido = mlist_find(listaEstados, (void*) esEncargado);
	t_Estado* nodoEncargado = (t_Estado*) nodoEncargadoObtenido;
	return nodoEncargado->archivoTemporal;
}


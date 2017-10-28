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


void init_job(t_serial *content);

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
					{//init_job(packetOperacion.content);
					t_yfile* Datosfile = malloc(sizeof(t_yfile));
					/*listaNodosActivos = mlist_create(); //luego borrar es para el hardcodeo
					Datosfile->blocks = mlist_create(); //luego borrar es para el hardcodeo */
					log_print("OP_INIT_JOB");
					requerirInformacionFilesystem(packetOperacion.content);
					reciboInformacionSolicitada(listaNodosActivos,Datosfile,sock);
					///////////////////////////// hardcodeado
					/*t_infoNodo* UnNodo = malloc(sizeof(t_infoNodo));
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
					mlist_append(Datosfile->blocks,bloque); */


					/////////////////////////////////
					//planifico
					int tamaniolistaNodos = mlist_length(listaNodosActivos);
					t_workerPlanificacion planificador[tamaniolistaNodos];
					planificar(planificador, tamaniolistaNodos,Datosfile->blocks);
					//inicio etapa de tranformacion
					enviarEtapa_transformacion_Master(tamaniolistaNodos,planificador,listaNodosActivos,Datosfile->blocks,sock);
					numeroJob +=1;
					}
					break;
				case OP_TRANSFORMACION_LISTA :
					{respuestaOperacionTranf* finalizoOperacion = serial_unpackRespuestaOperacion(packetOperacion.content);
					if(finalizoOperacion->response == -1){
						//replanifacion();
						//actualizoTablaEstado(finalizoOperacion.nodo,finalizoOperacion.bloque,"Transformacion");hay que agregar una entrada para el nuevo nodo que va a ejecutar
					}
					else{
						int job = 0; //cambiar deberia venir de master
						actualizoTablaEstado(finalizoOperacion->nodo,finalizoOperacion->bloque,sock,job,"FinalizadoOK");
						if(verificoFinalizacionTransformacion(finalizoOperacion->nodo,sock,job)){
							t_infoNodo* IP_PUERTOnodo = BuscoIP_PUERTO(finalizoOperacion->nodo);
							mlist_t* archivosTemporales_Transf = BuscoArchivosTemporales(finalizoOperacion->nodo,sock,job);
							char* temporal_local = generarNombreTemporal_local(finalizoOperacion->nodo,sock);
							mandarEtapaReduccionLocal(sock,finalizoOperacion->nodo,IP_PUERTOnodo,archivosTemporales_Transf,temporal_local);
						}
					}
					}
				break;
				case OP_REDUCCION_LOCAL_LISTA:
					{respuestaOperacionRL* finalizoRL = serial_unpackrespuestaOperacionRL(packetOperacion.content);
					int job = 0; //cambiar
					if(finalizoRL->response == -1){

						actualizoTablaEstado(finalizoRL->nodo,-1,sock,job,"Error");
						abortarJob(sock);
					}
					else{
						actualizoTablaEstado(finalizoRL->nodo,-1,sock,job,"FinalizadoOK");
						if(verificoFinalizacionRl(finalizoRL->nodo,job)){
							mandarEtapaReduccionGL(sock,job);
						}
					}
					}
				break;
				default:
					log_report("Operación desconocida: %s", packetOperacion.operation);
				break;

				}
				printf("llegue aca \n");
				//serial_destroy(packetOperacion.content);
			}
		}
	}
}

void init_job(t_serial *content) {
	char *file = mstring_empty(NULL);
	serial_unpack(content, "s", &file);
	log_print("Comenzando tarea para archivo %s", file);
	free(file);
}


void agregarAtablaEstado(char* nodo,int Master,int bloque,char* etapa,char* archivo_temporal,char* estado){
	t_Estado*  nuevoEstado = malloc(sizeof(t_Estado));
	nuevoEstado->job = numeroJob;
	nuevoEstado->master = Master;
	nuevoEstado->nodo = nodo;
	nuevoEstado->block = bloque;
	nuevoEstado->etapa = etapa;
	nuevoEstado->archivoTemporal = archivo_temporal;
	nuevoEstado->estado = estado;
	mlist_append(listaEstados,nuevoEstado);

}

void requerirInformacionFilesystem(t_serial *file){
	//t_serial *content = serial_pack("s", file->data); //debo enviar un char* no un serial
	t_packet packetInfoFs = protocol_packet(OP_REQUEST_FILE_INFO, file);
	printf("operacion a enviar %d \n", packetInfoFs.operation);
	protocol_send_packet(packetInfoFs,yama.fs_socket); //ROMPE ESTA LINEA en el socket send bytes
	printf("Ya envie la solicitud \n ");
	serial_destroy(packetInfoFs.content);
}


void reciboInformacionSolicitada(mlist_t* listaNodosActivos,t_yfile* Datosfile,int master){
	t_packet packetNodosActivos = protocol_receive_packet(yama.fs_socket);
	if(packetNodosActivos.operation == OP_NODES_ACTIVE_INFO){
		printf("recibi la primer info \n");
	listaNodosActivos = nodelist_unpack(packetNodosActivos.content);
	}
		t_packet packetArchivo = protocol_receive_packet(yama.fs_socket);
		if(packetArchivo.operation == OP_ARCHIVO_INEXISTENTE){
			envioMasterErrorArchivo(master);
		}

		else if(packetArchivo.operation == OP_ARCHIVO_NODES){
			printf("recibi la segunda info \n");
			Datosfile = yfile_unpack(packetArchivo.content);
		}

}

void envioMasterErrorArchivo(int master){
	t_packet packetError = protocol_packet(OP_ERROR_JOB, serial_pack("i",ARCHIVO_INEXISTENTE));
	protocol_send_packet(packetError, master);
	serial_destroy(packetError.content);

}

void enviarEtapa_transformacion_Master(int tamaniolistaNodos,t_workerPlanificacion planificador[],mlist_t* listaNodosActivos,mlist_t* listabloques,int sock){
	mlist_t* lista = mlist_create();
	int i, j = 0;
	int nroIndex;
	for(i = 0; i < tamaniolistaNodos; i++){
		for(j = 0; j < mlist_length(planificador[i].bloque); j++){
			void* nroIndexOBtenido = mlist_get(planificador[i].bloque, j);
			nroIndex = (int)nroIndexOBtenido; //error en el numero de bloque devuelto

		  t_infoNodo* datosNodoAEnviar = mlist_get(listaNodosActivos, i);

		  bool condition(void* datosDeUnBloque){
		  	return ((t_block *) datosDeUnBloque)->index == nroIndex ? true : false;
		  }
		  void* datosDeUnBloqueObtenido = mlist_find(listabloques,(void*) condition); //devuelve void* hay que machear
		  t_block* datosDeUnBloque = (t_block*)datosDeUnBloqueObtenido;
		  int nroBloque;
		  if(!strcmp(datosDeUnBloque->copies[0].node, datosNodoAEnviar->nodo)){
			  nroBloque = datosDeUnBloque->copies[0].blockno;
		  } else{
			  nroBloque = datosDeUnBloque->copies[1].blockno;
		  }
		  char* nombreArchivoTemporal = malloc(sizeof(char)*21);
		  strcpy(nombreArchivoTemporal, generarNombreArchivoTemporalTransf(sock, nroBloque));
		  tEtapaTransformacion* et = new_etapa_transformacion(datosNodoAEnviar->nodo,datosNodoAEnviar->ip,datosNodoAEnviar->puerto,nroBloque, datosDeUnBloque->size,nombreArchivoTemporal); //ROMPE EN ESTA funcion
		  mlist_append(lista,et);
		  agregarAtablaEstado(datosNodoAEnviar->nodo,sock,nroBloque,"Transformacion",nombreArchivoTemporal,"En proceso");
		}
	}
	mandar_etapa_transformacion(lista,sock);

}

char* generarNombreArchivoTemporalTransf(int master, int bloque){
	char* nombreArchivoTemporal = malloc(sizeof(char)*21);
 sprintf(nombreArchivoTemporal,"/tmp/j%dMaster%d-temp%d",numeroJob,master,bloque);
 return nombreArchivoTemporal;
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

void mandarEtapaReduccionLocal(int socket,char* nodo,t_infoNodo* nodo_worker,mlist_t* archivos_transf,char* archivoTemporal_local){
	tEtapaReduccionLocal* etapaRL = new_etapa_rl(nodo,nodo_worker->ip,nodo_worker->puerto,archivos_transf,archivoTemporal_local);
	agregarAtablaEstado(nodo,socket,-1,"ReduccionLocal",archivoTemporal_local,"En proceso");
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


char* generarNombreTemporal_local(char* nodo,int master){
	int job = 0;
	char* nombreArchivoTemporal = malloc(sizeof(char)*21);
	sprintf(nombreArchivoTemporal,"/tmp/J%dMaster%d-%s",job,master,nodo);
	return nombreArchivoTemporal;

}

void abortarJob(int socketMaster){
	t_serial *serial = serial_pack("ERROR_REDUCCION_LOCAL");
	t_packet packet = protocol_packet(OP_ERROR_JOB,serial);
	protocol_send_packet(packet, socketMaster);
	serial_destroy(serial);
}


bool verificoFinalizacionRl(char* nodo,int job){
	return true;
}

void mandarEtapaReduccionGL(int master,int job){
	/*mlist_t* listaRG = mlist_create();
	mlist_t* NodosRL = BuscoNodos(master,job);
	char* nodo = seleccionarEncargado(NodosRL);
	int i;
	for(i=0; i< mlist_length(NodosRL); i++){
	void* unNodo_sendObtenido	= mlist_get(NodosRL,i);
	t_nodotemporalRL* unNodo_send = (t_nodotemporalRL*) unNodo_sendObtenido;
	t_infoNodo* infoNodo = BuscoIP_PUERTO(unNodo_send->nodo);
	if(string_equals_ignore_case(unNodo_send->nodo,nodo)){
	char* nombreRG = generarArchivoRG();
	tEtapaReduccionGlobal* etapaRG = new_etapa_rg(unNodo_send->nodo,infoNodo->ip,infoNodo->puerto,unNodo_send->archivoTemporalLocal,nombreRG,"SI");
	mlist_append(listaRG,etapaRG);
	}
	else{
		tEtapaReduccionGlobal* etapaRG = new_etapa_rg(unNodo_send->nodo,infoNodo->ip,infoNodo->puerto,unNodo_send->archivoTemporalLocal,"-1","NO");
		mlist_append(listaRG,etapaRG);
	}
	}
	mandar_etapa_rg(listaRG,master);
	*/
}

mlist_t* BuscoNodos(int master, int job){

	bool NodosReadyRL(void* unEstadoObtenido){
		t_Estado* Estado = (t_Estado*) unEstadoObtenido;
		return Estado->master == master && Estado->job == job && string_equals_ignore_case(Estado->etapa, "ReduccionLocal") ? true : false;
	}

	mlist_t* listaFiltrada = mlist_filter(listaEstados,(void*) NodosReadyRL);

	t_nodotemporalRL* getNodoTemporal(void* unEstadoObtenido){
			t_Estado* Estado;
			t_nodotemporalRL* info_enviar_toRG = malloc(sizeof(t_nodotemporalRL));
			Estado = (t_Estado*) unEstadoObtenido;
			info_enviar_toRG->nodo = Estado->nodo;
			info_enviar_toRG->archivoTemporalLocal = Estado->archivoTemporal;
			return info_enviar_toRG;
		}

	mlist_t* Nodos = mlist_map(listaFiltrada, (void*) getNodoTemporal);
	return Nodos;
}

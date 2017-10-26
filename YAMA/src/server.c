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
			} else {
				t_packet packet = protocol_receive_packet(sock);
				if(packet.operation == OP_UNDEFINED) {
					socket_close(sock);
					socket_set_remove(sock, &sockets);
					continue;
				}

				switch(packet.operation) {
				case OP_INIT_JOB:
					{init_job(packet.content);
					t_yfile Datosfile;
					listaNodosActivos = mlist_create(); //luego borrar es para el hardcodeo
					Datosfile.blocks = mlist_create(); //luego borrar es para el hardcodeo
					log_print("OP_INIT_JOB");
					/*requerirInfoNodos();
					reciboInfoNodos(listaNodosActivos);
					requerirInfoArchivo(packet.content);
					reciboInfoArchivo(&Datosfile);*/
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
					mlist_append(Datosfile.blocks,bloque);


					/////////////////////////////////
					//planifico
					int tamaniolistaNodos = mlist_length(listaNodosActivos);
					t_workerPlanificacion planificador[tamaniolistaNodos];
					planificar(planificador, tamaniolistaNodos,Datosfile);
					//inicio etapa de tranformacion
					enviarEtapa_transformacion_Master(tamaniolistaNodos,planificador,listaNodosActivos,Datosfile.blocks,sock);
					numeroJob +=1;
					}
					break;
				case OP_TRANSFORMACION_LISTA :
					{respuestaOperacion* finalizoOperacion = serial_unpackRespuestaOperacion(packet.content);
					if(finalizoOperacion->response == -1){
						//replanifacion();
						//actualizoTablaEstado(finalizoOperacion.nodo,finalizoOperacion.bloque,"Transformacion");hay que agregar una entrada para el nuevo nodo que va a ejecutar
					}
					else{
						actualizoTablaEstado(finalizoOperacion->nodo,finalizoOperacion->bloque,"FinalizadoOK");
						if(verificoFinalizacionTransformacion(finalizoOperacion->nodo,sock)){
							t_infoNodo* IP_PUERTOnodo = BuscoIP_PUERTO(finalizoOperacion->nodo);
							mlist_t* archivosTemporales_Transf = BuscoArchivosTemporales(finalizoOperacion->nodo,sock);
							char* temporal_local = generarNombreTemporal_local(finalizoOperacion->nodo,sock);
							mandarEtapaTransformacionLocal(sock,finalizoOperacion->nodo,IP_PUERTOnodo,archivosTemporales_Transf,temporal_local);
						}
					}
					}
				break;
				default:
					log_report("Operación desconocida: %s", packet.operation);
				break;

				}

				serial_destroy(packet.content);
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

void requerirInfoArchivo(t_serial *file){
	t_serial *content = serial_pack("s", file);
	t_packet packet = protocol_packet(OP_GET_FILE, content);
	protocol_send_packet(packet,yama.fs_socket);
}


void requerirInfoNodos(){
	protocol_send_response(yama.fs_socket,OP_NODE_INFO);
}

void reciboInfoNodos(mlist_t* listaNodosActivos){
	t_packet packet = protocol_receive_packet(yama.fs_socket);
	listaNodosActivos = nodelist_unpack(packet.content);

}

void reciboInfoArchivo(t_yfile* Datosfile){
	t_packet packet = protocol_receive_packet(yama.fs_socket);
	if(packet.operation == OP_GET_FILE){
		Datosfile = yfile_unpack(packet.content);
	}
}

void enviarEtapa_transformacion_Master(int tamaniolistaNodos,t_workerPlanificacion planificador[],mlist_t* listaNodosActivos,mlist_t* listabloques,int sock){
	mlist_t* lista = mlist_create();
	int i, j = 0;
	for(i = 0; i < tamaniolistaNodos; i++){
		for(j = 0; j < mlist_length(planificador[i].bloque); j++){
			void* nroBloqueObtenido = mlist_get(planificador[i].bloque, j);
			int  nroBloque = (int)nroBloqueObtenido;

		  t_infoNodo* datosNodoAEnviar = mlist_get(listaNodosActivos, i);

		  bool condition(void* datosDeUnBloque){
		  	return ((t_block *) datosDeUnBloque)->index == nroBloque ? true : false;
		  }
		  t_block * datosDeUnBloque = mlist_find(listabloques,(void*) condition);
		  char* nombreArchivoTemporal = malloc(sizeof(char)*21);
		  strcpy(nombreArchivoTemporal, generarNombreArchivoTemporal(sock, nroBloque));
		  tEtapaTransformacion* et = new_etapa_transformacion(datosNodoAEnviar->nodo,datosNodoAEnviar->ip,datosNodoAEnviar->puerto,nroBloque, datosDeUnBloque->size,nombreArchivoTemporal);
		  mlist_append(lista,et);
		  agregarAtablaEstado(datosNodoAEnviar->nodo,sock,nroBloque,"Transformacion",nombreArchivoTemporal,"En proceso");
		}
	}
	mandar_etapa_transformacion(lista,sock);

}

char* generarNombreArchivoTemporal(int master, int bloque){
 char* nombreArchivoTemporal = malloc(sizeof(char)*21);
 sprintf(nombreArchivoTemporal,"/tmp/Master%d-temp%d",master,bloque);
 return nombreArchivoTemporal;
}


void actualizoTablaEstado(char* nodo,int bloque,char* estado){
	t_Estado* estadoActual;
	t_Estado* nuevoEstado = malloc(sizeof(t_Estado));
	bool condition(void* estadoTarea){
	  	return ((t_Estado *) estadoTarea)->nodo == nodo && ((t_Estado *) estadoTarea)->block == bloque ? true : false;
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

bool verificoFinalizacionTransformacion(char* nodo,int socket){
	bool esNodoBuscado(void* estadoTarea){
		  	return !strcmp(((t_Estado *) estadoTarea)->nodo,nodo) && ((t_Estado *) estadoTarea)->master == socket && strcmp(((t_Estado *) estadoTarea)->estado, "Error");
	}

	mlist_t* listaFiltradaDelNodo = mlist_filter(listaEstados, (void*)esNodoBuscado);

	bool FinalizacionDeTransf_Nodo(void* estadoTarea){
			  	return !strcmp(((t_Estado *) estadoTarea)->estado,"TerminadoOK");
	}


	return mlist_all(listaFiltradaDelNodo, (void*) FinalizacionDeTransf_Nodo);

}

void mandarEtapaTransformacionLocal(int socket,char* nodo,t_infoNodo* nodo_worker,mlist_t* archivos_transf,char* archivoTemporal_local){
	tEtapaReduccionLocal* etapaRL = new_etapa_rl(nodo,nodo_worker->ip,nodo_worker->puerto,archivos_transf,archivoTemporal_local);
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

mlist_t* BuscoArchivosTemporales(char* nodo,int socket){

	bool esNodoBuscado(void* estadoTarea){
		  	return !strcmp(((t_Estado *) estadoTarea)->nodo,nodo) && ((t_Estado *) estadoTarea)->master == socket && !strcmp(((t_Estado *) estadoTarea)->estado, "FinalizadoOK");
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
	char* nombreArchivoTemporal = malloc(sizeof(char)*21);
	sprintf(nombreArchivoTemporal,"/tmp/Master%d-%s",master,nodo);
	return nombreArchivoTemporal;

}

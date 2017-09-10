#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <commons/config.h>
#include <commons/log.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "socket.h"
#include "serial.h"
#include <string.h>

#include <protocol.h>

#define MAXIMO_TAMANIO_DATOS 256 //definiendo el tamanio maximo
int SocketBuscado_GLOBAL = 0;
t_log* logYama;

typedef struct{
 char* FS_IP;
 char* FS_PUERTO;
 char* RETARDO_PLANIFICACION;
 char* ALGORITMO_BALANCEO;
 char* MASTER_PUERTO;
}tinformacion;

typedef struct {
 int id;
 int tamanio;
} t_cabecera;
int socket_FileSystem;

//funciones
//void escucharPuertosDataNodeYYama(tinformacion);
void escucharPuertosMaster(tinformacion);
void handshakeConMaster(int, struct sockaddr_in, fd_set, fd_set, int);
void handshakeConFS();

int main() {
	char* path = "/home/utnso/archivoConfiguracion/archivoConfigYama.cfg";
	t_config* archivoConfig = config_create(path);
	tinformacion informacion_socket;
	struct sockaddr_in addr_FileSystem;

	logYama = log_create("logYama.log", "Yama", false,LOG_LEVEL_TRACE);
	//VALIDACIONES

	if (archivoConfig == NULL) {
		log_error(logYama,"Archivo configuracion: error al intentar leer ruta");
		return 1;
	}
	if (!config_has_property(archivoConfig, "FS_IP")) {
		log_error(logYama,"Archivo configuracion: error al leer IP_FILESYSTEM");
		return 1;
	}
	if (!config_has_property(archivoConfig, "FS_PUERTO")) {
		log_error(logYama, "Archivo configuracion: error al leer FS_PUERTO");
		return 1;
	}
	if (!config_has_property(archivoConfig, "RETARDO_PLANIFICACION")) {
		log_error(logYama, "Archivo configuracion: error al leer RETARDO_PLANIFICACION");
		return 1;
	}
	if (!config_has_property(archivoConfig, "ALGORITMO_BALANCEO")) {
		log_error(logYama, "Archivo configuracion: error al leer ALGORITMO_BALANCEO");
		return 1;
	}
	if (!config_has_property(archivoConfig, "MASTER_PUERTO")) {
		log_error(logYama, "Archivo configuracion: error al leer MASTER_PUERTO");
		return 1;
	}
	//fin VALIDACIONES
	//configuracion del archivo configuracion
	informacion_socket.FS_IP = config_get_string_value(archivoConfig, "FS_IP");
	informacion_socket.FS_PUERTO = config_get_string_value(archivoConfig, "FS_PUERTO");
	informacion_socket.RETARDO_PLANIFICACION = config_get_string_value(archivoConfig, "RETARDO_PLANIFICACION");
	informacion_socket.ALGORITMO_BALANCEO = config_get_string_value(archivoConfig, "ALGORITMO_BALANCEO");
	informacion_socket.MASTER_PUERTO = config_get_string_value(archivoConfig, "MASTER_PUERTO");

	//me conecto con filesystem
	socket_FileSystem = crearSocket();
	inicializarSOCKADDR_IN(&addr_FileSystem,informacion_socket.FS_IP,informacion_socket.FS_PUERTO);

	if(connect(socket_FileSystem,(struct sockaddr*) &addr_FileSystem,sizeof(struct sockaddr)) == -1){
		perror("Error en la conexion con el FileSystem");
	}

	handshakeConFS();
	escucharPuertosMaster(informacion_socket);
	return 0;
}


void escucharPuertosMaster(tinformacion informacion_socket) {
	fd_set master;
	fd_set read_fds;
	int maximo_Sockets;
	//master
	int socketEscuchandoMaster;
	struct sockaddr_in direccion_Master;

	//crea socket para el master
	inicializarSOCKADDR_IN(&direccion_Master,informacion_socket.FS_IP,informacion_socket.MASTER_PUERTO);
	socketEscuchandoMaster = crearSocket(); //abstraccion, se debe armar la funcion

	//socket servidor
	reutilizarSocket(socketEscuchandoMaster); // obviar el mensaje "address already in use"
	asignarDirecciones(socketEscuchandoMaster, (struct sockaddr *) &direccion_Master); // asignamos el Struct de las direcciones al socket

	//Preparo para escuchar los 2 puertos en el master
	printf("Esperando conexiones entrantes \n");
	FD_ZERO(&master);
	FD_SET(socketEscuchandoMaster, &master);
	maximo_Sockets = socketEscuchandoMaster;

	if (listen(socketEscuchandoMaster, 10) == -1) {
		perror("listen");
		exit(1);
	}

	//Comienza el Select
	for (;;) {
		int nuevoSocket;
		FD_ZERO(&read_fds);
		struct sockaddr_in remoteaddr;
		socklen_t addrlen;
		read_fds = master; // cópialo
		if (select((maximo_Sockets) + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}

		if (FD_ISSET(socketEscuchandoMaster, &read_fds)) { // ¡¡tenemos datos de un Master!!
			printf("Escucho algo por Master \n");
			addrlen = sizeof(remoteaddr);
			if ((nuevoSocket = accept(socketEscuchandoMaster, (struct sockaddr *) &remoteaddr, (socklen_t *) &addrlen)) == -1) {
				perror("Error de aceptacion de un master \n");
			} else {
				FD_SET(nuevoSocket, &master); // añadir al conjunto maestro
				FD_SET(nuevoSocket, &read_fds);
				maximo_Sockets = (maximo_Sockets < nuevoSocket) ? nuevoSocket : maximo_Sockets;
				printf("Ya acepte un master \n ");
				handshakeConMaster(nuevoSocket, remoteaddr, master, read_fds, maximo_Sockets);
				//ejecuto lo que tengo que hacerr para el master....
			}
		}
		else {
			/*Si no entro en los if anteriores es por que no es conexion nueva por
			 ninguno de los dos sockets, por lo tanto es algun socket descriptor
			 ya almacenado que tiene actividad.
			 luego gestionar mensaje*/
			int socketFor;
			int bytes;
			t_cabecera header;
			for (socketFor = 0; socketFor < (maximo_Sockets + 1); socketFor++) {
				if (FD_ISSET(socketFor, &read_fds)) {
					if ((bytes = recv(socketFor, &header, sizeof(header), 0)) //recibe un mensaje el cual en caso de error es menor a cero y se limpian el FD y se cierra el socket
							<= 0) {
						FD_CLR(socketFor, &read_fds);
						FD_CLR(socketFor, &master);
						close(socketFor);
					} else {
						///switch con mensajes
					}
				}
			}

		}
	}
}

void handshakeConMaster(int nuevoSocket, struct sockaddr_in remoteaddr, fd_set master, fd_set read_fds, int maximo_Sockets) {
	char buffer_select[MAXIMO_TAMANIO_DATOS];
	int cantidadBytes;

	cantidadBytes = recv(nuevoSocket, buffer_select, sizeof(buffer_select), 0);
	buffer_select[cantidadBytes] = '\0';
	printf("Credencial recibida: %s\n", buffer_select);
	if (!strcmp(buffer_select, "Yatpos-Master\0")) { //Yatpos es la credencial que autoriza al proceso a seguir conectado
		printf("Yama: nueva conexion desde %s en socket %d\n", inet_ntoa(remoteaddr.sin_addr), nuevoSocket);
		if (send(nuevoSocket, "Bienvenido a Yama!", 20, 0) == -1) {
			perror("Error en el send");
		}
	} else {
		printf("El proceso que requirio acceso, no posee los permisos adecuados\n");
		if (send(nuevoSocket, "Usted no esta autorizado!", MAXIMO_TAMANIO_DATOS, 0) == -1) {
			perror("Error en el send");
		}
		close(nuevoSocket);
	}
}

void handshakeConFS(){
	 char handshake[26];

	 if((send(socket_FileSystem,"Yatpos-Yama",sizeof("Yatpos-Yama"),0)) <= 0) //envio credencial
	 {
	  perror("No pudo enviar!");
	  exit(1);
	 }
	 if ((recv(socket_FileSystem,handshake,26,0)) <= 0) //"Bienvenido al FileSystem!"
	 {
	  perror("El FileSystem se desconectó");
	  exit(1);
	 }
	 handshake[26] = '\0';
	 printf("%s\n",handshake);
}


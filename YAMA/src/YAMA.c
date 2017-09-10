#include <commons/log.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <process.h>
#include <config.h>
#include <file.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

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
void escucharPuertosMaster();
void handshakeConMaster(int, struct sockaddr_in);
void handshakeConFS();

int main() {
	struct sockaddr_in addr_FileSystem;
	process_init(PROC_YAMA);
	config_load();
	logYama = log_create("logYama.log", "Yama", false,LOG_LEVEL_TRACE);
	//me conecto con filesystem
	socket_FileSystem = crearSocket();
	inicializarSOCKADDR_IN(&addr_FileSystem,(char*) config_get("FS_IP"),(char*) config_get("FS_PUERTO"));

	if(connect(socket_FileSystem,(struct sockaddr*) &addr_FileSystem,sizeof(struct sockaddr)) == -1){
			perror("Error en la conexion con el FileSystem");
	}

	handshakeConFS();
	////// me quedo a la escucha
	escucharPuertosMaster();
	return 0;
}


void escucharPuertosMaster() {
	fd_set master;
	fd_set read_fds;
	int maximo_Sockets;
	//master
	int socketEscuchandoMaster;
	struct sockaddr_in direccion_Master;

	//crea socket para el master
	inicializarSOCKADDR_IN(&direccion_Master,(char*) config_get("FS_IP"),(char*) config_get("MASTER_PUERTO"));
	socketEscuchandoMaster = crearSocket();

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
				handshakeConMaster(nuevoSocket, remoteaddr);
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

void handshakeConMaster(int nuevoSocket, struct sockaddr_in remoteaddr) {
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


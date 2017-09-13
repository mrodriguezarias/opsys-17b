#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <process.h>
#include <serial.h>
#include <socket.h>

typedef enum {
	OP_UNDEFINED,
	OP_HANDSHAKE,

	INICIAR_TRANSFORMACION = 3,        //yama -> master
  	INICIAR_REDUCCION_LOCAL = 4,	 //yama -> master
	INICIAR_REDUCCION_GLOBAL = 5, 	//yama -> master
	INICIAR_ALMACENAMIENTO = 6,		//yama -> master

	TRANSFORMACION_LISTA = 7,  //master->yama
	REDUCCION_LOCAL_LISTA = 8, //master->yama
	REDUCCION_GLOBAL_LISTA = 9,//master->yama

	GETBLOQUE, //Filesystem -> Datanode
	SETBLOQUE, //Filesystem -> Datanode

} t_operation;

typedef struct {
	t_process sender;		// Proceso remitente
	t_operation operation;	// Operación a realizar
	t_serial content;		// Contenido serializado
} t_packet;

/**
 * Crea un paquete para enviar una operación.
 * @param operation Operación a realizar.
 * @param content Contenido serializado.
 * @return Paquete.
 */
t_packet protocol_packet(t_operation operation, t_serial content);

/**
 * Envía un packete a un determinado socket.
 * @param packet Paquete.
 * @param socket Descriptor del socket.
 */
void protocol_send(t_packet packet, t_socket socket);

/**
 * Recibe un paquete de un determinado socket.
 * Si se recibe contenido, luego de usarlo debe ser liberado con free().
 * @param socket Descriptor del socket.
 * @return Paquete.
 */
t_packet protocol_receive(t_socket socket);

/**
 * Envía un apretón de manos.
 * @param socket Descriptor del socket.
 */
void protocol_handshake(t_socket socket);

#endif /* PROTOCOL_H_ */

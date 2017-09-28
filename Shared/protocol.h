#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <process.h>
#include <serial.h>
#include <socket.h>

typedef enum {
	OP_UNDEFINED,
	OP_HANDSHAKE,
	OP_INIT_JOB,

	OP_INICIAR_TRANSFORMACION,		// yama -> master
  	OP_INICIAR_REDUCCION_LOCAL,		// yama -> master
	OP_INICIAR_REDUCCION_GLOBAL,	// yama -> master
	OP_INICIAR_ALMACENAMIENTO,		// yama -> master

	OP_TRANSFORMACION_LISTA,		// master -> yama
	OP_REDUCCION_LOCAL_LISTA,		// master -> yama
	OP_REDUCCION_GLOBAL_LISTA,		// master -> yama

	OP_GETBLOQUE,					// filesystem -> datanode
	OP_SETBLOQUE,					// filesystem -> datanode

	REGISTRARNODO,					// filesystem -> datanode

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

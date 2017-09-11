#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include "socket.h"
#include "process.h"

typedef enum {
	OP_UNDEFINED,
	OP_HANDSHAKE,
} t_operation;

typedef struct {
	t_process sender;		// Proceso remitente
	t_operation operation;	// Operación a realizar
	size_t size;			// Tamaño del paquete
	char *content;			// Contenido serializado
} t_packet;

/**
 * Crea un paquete para enviar una operación.
 * @param header Encabezado del paquete.
 * @param payload (Opcional) Cuerpo del paquete.
 * @return Paquete.
 */
t_packet protocol_packet(t_operation operation, size_t size, char *content);

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

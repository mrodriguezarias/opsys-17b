#ifndef SERIAL_H_
#define SERIAL_H_

#include <stddef.h>
#include "mlist.h"

#define SERIAL_MAX 1024

typedef struct serial {
	void *data;
	size_t size;
} t_serial;

/*
 * Referencias para el formato de serialización:
 * ---
 * c : char
 * h : short
 * i : int, long
 * l : long long
 * f : float
 * d : double
 * s : string (char*)
 * x : binary (t_serial*)
 * ---
 * Letra minúscula : signed
 * Letra mayúscula : unsigned
 */

/**
 * Crea una estructura de datos serializados.
 * @param data Datos.
 * @param size Tamaño de los datos.
 */
t_serial *serial_create(void *data, size_t size);

/**
 * Clona un contenedor serial, copiando su contenido.
 * @param serial Contenedor original.
 * @return Contenedor copia.
 */
t_serial *serial_clone(t_serial *serial);

/**
 * Serializa datos y los agrega a un contenedor serial.
 * @param serial Contenedor serial.
 * @param format Formato de serialización.
 */
void serial_add(t_serial *serial, const char *format, ...);

/**
 * Deserializa datos y los remueve de un contenedor serial.
 * @param serial Contenedor serial.
 * @param format Formato de serialización.
 */
void serial_remove(t_serial *serial, const char *format, ...);

/*
 * Serializa datos según el formato especificado.
 * @return Datos serializados.
 */
t_serial *serial_pack(const char *format, ...);

/*
 * Deserializa datos según el formato especificado.
 * Elimina el contenedor de los datos.
 * @param serial Datos serializados.
 */
void serial_unpack(t_serial *serial, const char *format, ...);

/**
 * Destruye una estructura de datos serializados.
 * @param serial Estructura serial.
 */
void serial_destroy(t_serial *serial);

#endif /* SERIAL_H_ */

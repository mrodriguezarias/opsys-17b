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
 * Destruye una estructura de datos serializados.
 * @param serial Estructura serial.
 */
void serial_destroy(t_serial *serial);

/*
 * Serializa datos según el formato especificado.
 * @return Datos serializados (serial.data debe ser liberado con free()).
 */
t_serial *serial_pack(const char *format, ...);

/*
 * Deserializa datos según el formato especificado.
 * @param serial Datos serializados.
 */
void serial_unpack(t_serial *serial, const char *format, ...);

#endif /* SERIAL_H_ */

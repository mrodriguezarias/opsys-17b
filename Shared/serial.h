#ifndef SERIAL_H_
#define SERIAL_H_

#include <stddef.h>

typedef struct serial {
	char *data;
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
 * s : string
 * ---
 * Letra minúscula : signed
 * Letra mayúscula : unsigned
 */

/*
 * Serializa datos según el formato especificado.
 * @return Datos serializados (serial.data debe ser liberado con free()).
 */
t_serial serial_pack(const char *format, ...);

/*
 * Deserializa datos según el formato especificado.
 * @param serial Datos serializados.
 */
void serial_unpack(t_serial serial, const char *format, ...);

#endif /* SERIAL_H_ */

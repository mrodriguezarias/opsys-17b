#ifndef SERIAL_H_
#define SERIAL_H_

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
 * @param buf Búfer de datos serializados.
 * @return Tamaño de los datos serializados.
 */
size_t serial_pack(char *buf, const char *format, ...);

/*
 * Deserializa datos según el formato especificado.
 * @param buf Búfer de datos serializados.
 */
void serial_unpack(const char *buf, const char *format, ...);

#endif /* SERIAL_H_ */

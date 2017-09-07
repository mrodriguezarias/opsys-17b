#ifndef MSTRING_H_
#define MSTRING_H_

#include <stdbool.h>

/**
 * Recorta una cadena, eliminando todos los espacios en sus extremos.
 * No crea una nueva cadena, sino que modifica la que recibe.
 * @param string Cadena a recortar.
 * @return Puntero a la cadena.
 */
char *mstring_trim(char *string);

/**
 * Crea una nueva cadena siguiendo el formato especificado.
 * Útil para concatenar cadenas. Por ejemplo:
 * mstring_format("Hola, %s!", nombre);
 * La cadena creada debe ser liberada usando free().
 * @param format Formato de la cadena.
 * @return Cadena con formato.
 */
char *mstring_format(const char *format, ...);

/**
 * Verifica si una cadena está vacía.
 * @param string Cadena a verificar.
 * @return Valor lógico indicando si la cadena está vacía.
 */
bool mstring_empty(const char *string);

#endif /* MSTRING_H_ */

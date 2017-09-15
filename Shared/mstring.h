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
char *mstring_create(const char *format, ...);

/**
 * Modifica una usando el formato especificado.
 * @param Puntero a la cadena a modificar.
 * @param format Formato de la cadena.
 */
void mstring_format(char **string, const char *format, ...);

/**
 * Verifica si una cadena está vacía.
 * @param string Cadena a verificar.
 * @return Valor lógico indicando si la cadena está vacía.
 */
bool mstring_empty(const char *string);

/**
 * Verifica si dos cadenas son iguales.
 * @param str1 Cadena 1.
 * @param str2 Cadena 2.
 * @return Valor lógico indicando si las cadenas son iguales.
 */
bool mstring_equal(const char *str1, const char *str2);

/**
 * Verifica si dos cadenas son iguales.
 * No tiene en cuenta mayúsculas y minúsculas. (i=ignore case)
 * @param str1 Cadena 1.
 * @param str2 Cadena 2.
 * @return Valor lógico indicando si las cadenas son iguales.
 */
bool mstring_equali(const char *str1, const char *str2);

/**
 * Convierte una cadena a un valor entero.
 * @param string Cadena a convertir.
 * @return Valor entero.
 */
int mstring_toint(const char *string);

#endif /* MSTRING_H_ */

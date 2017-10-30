#ifndef MSTRING_H_
#define MSTRING_H_

#include <stdbool.h>
#include <alloca.h>

// Tamaño máximo de buffer.
#define MSTRING_MAXSIZE 1024

// Devuelve un buffer que se libera automáticamente al finalizar la función que lo llamó.
#define mstring_buffer() ((char*)alloca(MSTRING_MAXSIZE))

/**
 * Crea una nueva cadena con el mismo contenido que otra.
 * @param string Cadena a copiar el contenido.
 * @return Puntero a la nueva cadena.
 */
char *mstring_duplicate(const char *string);

/**
 * Devuelve el tamaño de una cadena.
 * @param string Cadena.
 * @return Tamaño.
 */
size_t mstring_length(const char *string);

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
 * Modifica una cadena usando el formato especificado.
 * @param Puntero a la cadena a modificar.
 * @param format Formato de la cadena.
 */
void mstring_format(char **string, const char *format, ...);

/**
 * Crea una cadena copiando parte de otra.
 * Excluye el carácter en el índice final.
 * Si el índice final es negativo, se lo hace relativo desde el final.
 * @param string Cadena a copiar.
 * @param start Índice inicial.
 * @param end Índice final.
 * @return Nueva cadena con la copia (debe liberarse con free()).
 */
char *mstring_copy(const char *string, int start, int end);

/**
 * Recorta una cadena hasta un determinado largo.
 * @param string Puntero a la cadena a recortar.
 * @param length Largo de la cadena.
 * @return Puntero a la cadena.
 */
char *mstring_crop(char **string, int length);

/**
 * Verifica si una cadena está vacía.
 * @param string Cadena a verificar.
 * @return Valor lógico indicando si la cadena está vacía.
 */
bool mstring_isempty(const char *string);

/**
 * Crea una cadena vacía o borra el contenido de una cadena existente.
 * @param string Puntero a la cadena.
 * @return La cadena vacía.
 */
char *mstring_empty(char **string);

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
 * Verifica si una cadena contiene a otra.
 * @param string Cadena contenedora.
 * @param substring Cadena a buscar.
 * @return Valor lógico indicando si se encontró la cadena.
 */
bool mstring_contains(const char *string, const char *substring);

/**
 * Devuelve un puntero a una subcadena dentro de una cadena.
 * @param string Cadena contenedora.
 * @param substring Cadena a buscar.
 * @return Puntero a la subcadena (NULL si no existe).s
 */
char *mstring_find(const char *string, const char *substring);

/**
 * Calcula la cantidad de veces que aparece una subcadena.
 * @param string Cadena contenedora.
 * @param substring Cadena a contar.
 * @return Veces que aparece la subcadena en la cadena.
 */
int mstring_count(const char *string, const char *substring);

/**
 * Devuelve el índice de una subcadena dentro de una cadena.
 * @param string Cadena contenedora.
 * @param substring Cadena a buscar.
 * @return Índice de la subcadena (-1 si no existe).
 */
int mstring_index(const char *string, const char *substring);

/**
 * Convierte una cadena a un valor entero.
 * @param string Cadena a convertir.
 * @return Valor entero.
 */
int mstring_toint(const char *string);

/**
 * Comparador de cadenas para ordenarlas de forma ascendente.
 * @param str1 Cadena 1.
 * @param str2 Cadena 2.
 * @return 1 si str1 es alfabéticamente menor que str2, 0 si es mayor.
 */
bool mstring_asc(const char *str1, const char *str2);

/**
 * Comparador de cadenas para ordenarlas de forma descendente.
 * @param str1 Cadena 1.
 * @param str2 Cadena 2.
 * @return 1 si str1 es alfabéticamente mayor que str2, 0 si es menor.
 */
bool mstring_desc(const char *str1, const char *str2);

/**
 * Repite una cadena un número determinado de veces.
 * @param string Cadena a repetir.
 * @param times Número de veces.
 * @return Cadena con la repetición (a liberar con free()).
 */
char *mstring_repeat(const char *string, int times);

/**
 * Reemplaza una subcadena de una cadena por otra cadena.
 * @param string Puntero a la cadena original.
 * @param substring Subcadena a reemplazar.
 * @param replacement Cadena de reemplazo.
 * @return Valor lógico indicando si se realizó el reemplazo.
 */
bool mstring_replace(char **string, const char *substring, const char *replacement);

/**
 * Verifica si una cadena tiene determinado prefijo.
 * @param string Cadena a verificar.
 * @param Prefijo.
 * @return Valor lógico con el resultado.
 */
bool mstring_hasprefix(const char *string, const char *prefix);

/**
 * Verifica si una cadena tiene determinado sufijo.
 * @param string Cadena a verificar.
 * @param Sufijo.
 * @return Valor lógico con el resultado.
 */
bool mstring_hassuffix(const char *string, const char *suffix);

/**
 * Devuelve un puntero al final de una cadena.
 * @param string Cadena.
 * @return Puntero al final de la cadena.
 */
char *mstring_end(const char *string);

/**
 * Agrega los prefijos a un tamaño digital.
 * @param size Tamaño en bytes.
 * @return Cadena resultante, a liberar con free().
 */
char *mstring_bsize(size_t size);

#endif /* MSTRING_H_ */

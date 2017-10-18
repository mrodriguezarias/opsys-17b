#ifndef DIRTREE_H_
#define DIRTREE_H_

#include <stdbool.h>
#include <stddef.h>

typedef struct {
	int index;
	char name[255];
	int parent;
} t_directory;

/**
 * Esta estructura se mantiene sincronizada y almacenada en un archivo
 * ubicado en la ruta ~/yatpos/metadata/directorios.dat.
 */

/**
 * Levanta la estructura del archivo o la crea si no existía.
 */
void dirtree_init(void);

/**
 * Determina la cantidad de entradas en la tabla de directorios.
 * @return Cantidad de directorios.
 */
int dirtree_size(void);

/**
 * Agrega los directorios de una ruta, incluyendo los intermedios si no existían.
 * @param path Ruta a agregar.
 * @return Último directorio de la ruta.
 */
t_directory *dirtree_add(const char *path);

/**
 * Busca un directorio en la tabla según su ruta.
 * @param path Ruta a buscar.
 * @return Puntero al directorio o NULL si no existía.
 */
t_directory *dirtree_find(const char *path);

/**
 * Verifica si existe un directorio en la tabla.
 * @param Ruta a verificar.
 * @return Valor lógico con el resultado.
 */
bool dirtree_contains(const char *path);

/**
 * Itera sobre todos los directorios aplicando una función a cada uno.
 * @param routine Función a aplicar.
 */
void dirtree_traverse(void (*routine)(t_directory *dir));

/**
 * Mueve un directorio.
 * @param path Ruta al directorio a mover.
 * @param new_path Nueva ruta del directorio.
 */
void dirtree_move(const char *path, const char *new_path);

/**
 * Renombra un directorio.
 * @param path Ruta al directorio a renombrar.
 * @param new_name Nuevo nombre.
 */
void dirtree_rename(const char *path, const char *new_name);

/**
 * Elimina un directorio de la tabla según su ruta.
 * Si tenía directorios hijos, los mismos también son eliminados.
 * @param Ruta a eliminar.
 */
void dirtree_remove(const char *path);

/**
 * Elimina todos los directorios.
 */
void dirtree_clear(void);

/**
 * Imprime el árbol de directorios.
 */
void dirtree_print(void);

/**
 * Devuelve la cantidad de hijos de un directorio.
 * @param path Ruta al directorio padre.
 * @return Cantidad de directorios.
 */
size_t dirtree_count(const char *path);

/**
 * Imprime los directorios hijos de un directorio.
 * @param path Ruta al directorio padre.
 */
void dirtree_list(const char *path);

/**
 * Crea una cadena con la ruta de los archivos de un directorio.
 * @param ypath Ruta al directorio yama.
 * @return Ruta real del directorio, a liberar con free().
 */
char *dirtree_rpath(const char *ypath);

/**
 * Crea una cadena con la ruta yamafs de un directorio.
 * @param rpath Ruta real al directorio.
 * @return Ruta yama del directorio, a liberar con free().
 */
char *dirtree_ypath(const char *rpath);

/**
 * Cierra el árbol de directorios, guardando los cambios en el archivo.
 */
void dirtree_term(void);

#endif /* DIRTREE_H_ */

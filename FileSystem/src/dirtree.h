#ifndef DIRTREE_H_
#define DIRTREE_H_

#include <stdbool.h>

typedef struct {
	int index;
	char name[255];
	int parent;
} t_directory;

/**
 * Carga la estructura de directorios del archivo metadata/directorios.dat,
 * o la crea si no existía el archivo.
 */
void dirtree_load(void);

/**
 * Determina la cantidad de entradas en la tabla de directorios.
 * @return Cantidad de directorios.
 */
int dirtree_size(void);

/**
 * Agrega los directorios de una ruta, incluyendo los intermedios si no existían.
 * @param path Ruta a agregar.
 */
void dirtree_add(const char *path);

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
 * Elimina un directorio de la tabla según su ruta.
 * Si tenía directorios hijos, los mismos también son eliminados.
 * @param Ruta a eliminar.
 */
void dirtree_remove(const char *name);

/**
 * Imprime el árbol de directorios.
 */
void dirtree_print(void);

/**
 * Elimina todos los directorios.
 */
void dirtree_clear(void);

/**
 * Guarda la estructura de directorios en el archivo metadata/directorios.dat.
 */
void dirtree_save(void);

#endif /* DIRTREE_H_ */

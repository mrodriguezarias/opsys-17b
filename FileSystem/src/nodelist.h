#ifndef NODELIST_H_
#define NODELIST_H_

#include <socket.h>
#include <stdbool.h>
#include <thread.h>
#include <bitmap.h>
#include "yfile.h"

typedef struct t_node {
	char *name;
	int total_blocks;
	int free_blocks;
	thread_t *handler;
	t_socket socket;
	t_bitmap *bitmap;
} t_node;

/**
 * Esta estructura se mantiene sincronizada y almacenada en un archivo
 * ubicado en la ruta ~/yatpos/metadata/nodos.bin.
 */

/**
 * Inicializa la lista de nodos, cargando un estado anterior si existía.
 */
void nodelist_init(void);

/**
 * Devuelve la cantidad de nodos en la lista de nodos.
 * @return Cantidad de nodos.
 */
int nodelist_size(void);

/**
 * Determina si un nodo está activo.
 * @param node Nodo.
 * @return Valor lógico con el resultado.
 */
bool nodelist_active(t_node *node);

/**
 * Devuelve el nodo de una determinada posición.
 * @param pos Posición.
 * @return Nodo de la posición.
 */
t_node * nodelist_get(int pos);

/**
 * Crea un nuevo nodo y lo agrega a la lista de nodos.
 * @param name Nombre del nodo a crear.
 * @param blocks Cantidad de bloques del nodo.
 * @return Puntero al nodo creado.
 */
t_node *nodelist_add(const char *name, int blocks);

/**
 * Busca un nodo en la lista de nodos según su ID.
 * @param name Nobre del nodo.
 * @return Puntero al nodo.
 */
t_node *nodelist_find(const char *name);

/**
 * Agrega un bloque a nodos de la lista.
 * Le envía el contenido del bloque al DataNode correspondiente.
 * @param block Bloque a agregar.
 * @param data Contenido del bloque.
 */
void nodelist_addblock(t_block *block, void *data);

/**
 * Elimina un nodo de la lista de nodos.
 * @param name Nombre del nodo a eliminar.
 */
void nodelist_remove(const char *name);

/**
 * Elimina todos los nodos de la lista de nodos.
 */
void nodelist_clear();

/**
 * Formatea todos los nodos de la lista de nodos.
 */
void nodelist_format();

/**
 * Imprime por pantalla la lista de nodos.
 */
void nodelist_print();

/**
 * Cierra la lista de nodos.
 */
void nodelist_term();

#endif /* NODELIST_H_ */

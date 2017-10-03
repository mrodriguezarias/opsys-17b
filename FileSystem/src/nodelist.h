#ifndef NODELIST_H_
#define NODELIST_H_

#include <socket.h>
#include <stdbool.h>

typedef struct {
	int id;
	int total_blocks;
	int free_blocks;
	bool available;
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
int nodelist_size();

/**
 * Crea un nuevo nodo y lo agrega a la lista de nodos.
 * @return Puntero al nodo creado.
 */
t_node *nodelist_add(void);

/**
 * Busca un nodo en la lista de nodos según su ID.
 * @param id ID del nodo.
 * @return Puntero al nodo.
 */
t_node *nodelist_find(int id);

/**
 * Determina si existe un nodo en la lista de nodos.
 * @param id ID del nodo.
 * @return Valor lógico con el resultado.
 */
bool nodelist_contains(int id);

/**
 * Elimina un nodo de la lista de nodos.
 * @param id ID del nodo a eliminar.
 */
void nodelist_remove(int id);

/**
 * Elimina todos los nodos de la lista de nodos.
 */
void nodelist_clear();

/**
 * Imprime por pantalla la lista de nodos.
 */
void nodelist_print();

#endif /* NODELIST_H_ */

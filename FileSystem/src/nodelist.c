#include "nodelist.h"
#include <mlist.h>
#include <stdlib.h>
#include <commons/config.h>
#include <path.h>
#include <file.h>
#include <system.h>
#include <mstring.h>

static mlist_t *nodes = NULL;
static t_config *config = NULL;

static const char *save_file_path(void);
static void init_config(void);
static void update_file(void);
static void add_node_to_file(t_node *node);
static void remove_node_from_file(t_node *node);

// ========== Funciones públicas ==========

void nodelist_init() {
	if(nodes != NULL) return;
	nodes = mlist_create();
	init_config();

	char **snodes = config_get_array_value(config, "NODOS");
	if(snodes == NULL) return;

	for(char **pnode = snodes; *pnode != NULL; pnode++) {
		t_node *node = malloc(sizeof(t_node));
		node->id = mstring_toint(*pnode);
		node->available = false;

		char *key = mstring_create("%sTotal", *pnode);
		node->total_blocks = config_get_int_value(config, key);
		mstring_format(&key, "%sLibre", *pnode);
		node->free_blocks = config_get_int_value(config, key);
		free(key);

		mlist_append(nodes, node);
	}
	free(snodes);
}

int nodelist_size() {
	return mlist_length(nodes);
}

t_node *nodelist_add() {
	t_node *node = calloc(1, sizeof(t_node));

	int find_max(int prev, t_node *node) {
		return node->id > prev ? node->id : prev;
	}
	node->id = mlist_reduce(nodes, find_max) + 1;
	node->available = true;
	mlist_append(nodes, node);
	add_node_to_file(node);
	return node;
}

t_node *nodelist_find(int id) {
	bool finder(t_node *elem) {
		return elem->id == id;
	}
	return mlist_find(nodes, finder);
}

bool nodelist_contains(int id) {
	return nodelist_find(id) != NULL;
}

void nodelist_remove(int id) {
	bool condition(t_node *elem) {
		return elem->id == id;
	}
	t_node *node = mlist_remove(nodes, condition, NULL);
	if(node == NULL) return;

	remove_node_from_file(node);
	free(node);
}

void nodelist_clear() {
	mlist_clear(nodes, free);
	dictionary_clean_and_destroy_elements(config->properties, free);
	update_file();
}

void nodelist_print() {
	puts("╭────┬────────────┬────────────────┬─────────────────╮");
	puts("│ ID │ Disponible │ Bloques libres │ Bloques totales │");
	if(mlist_length(nodes) == 0) {
		puts("├────┴────────────┴────────────────┴─────────────────┤");
		char *rpadding = mstring_repeat(" ", 19);
		printf("│%20sNo hay nodos.%s│\n", "", rpadding);
		puts("╰────────────────────────────────────────────────────╯");
		free(rpadding);
		return;
	}
	puts("├────┼────────────┼────────────────┼─────────────────┤");
	void iterator(t_node *node) {
		printf("│ %2i │ %s         │ %14i │ %15i │ \n", node->id, node->available ? "Sí" : "No"
				, node->free_blocks, node->total_blocks);
	}
	mlist_traverse(nodes, iterator);
	puts("╰────┴────────────┴────────────────┴─────────────────╯");
}

// ========== Funciones privadas ==========

static const char *save_file_path() {
	static char *path = NULL;
	if(path == NULL) {
		path = mstring_create("%s/metadata/nodos.bin", system_userdir());
	}
	return path;
}

static void init_config() {
	if(config != NULL) return;
	path_create(save_file_path());
	config = config_create((char*) save_file_path());
	if(config_has_property(config, "NODOS")) return;

	config_set_value(config, "TAMANIO", "0");
	config_set_value(config, "LIBRE", "0");
	config_set_value(config, "NODOS", "[]");
	config_save(config);
}

static void update_file() {
	char *keys[] = {"TAMANIO", "LIBRE", "NODOS"};
	char *key, *value = mstring_empty(NULL);

	int adder(int total, t_node *elem) {
		return total + (mstring_equal(key, "LIBRE") ? elem->free_blocks : elem->total_blocks);
	}

	for(char **pkey = keys; key = *pkey, !mstring_equal(key, "NODOS"); pkey++) {
		mstring_format(&value, "%i", mlist_reduce(nodes, adder));
		config_set_value(config, key, value);
	}

	char *formatter(t_node *elem) {
		return mstring_create("Nodo%i", elem->id);
	}

	free(value);
	value = mlist_tostring(nodes, formatter);
	config_set_value(config, key, value);

	free(value);
	config_save(config);
}

static void add_node_to_file(t_node *node) {
	char *key = mstring_create("Nodo%iTotal", node->id);
	char *value = mstring_create("%i", node->total_blocks);
	config_set_value(config, key, value);

	mstring_replace(&key, "Total", "Libre");
	mstring_format(&value, "%i", node->free_blocks);
	config_set_value(config, key, value);

	free(key);
	free(value);
	update_file();
}

static void remove_node_from_file(t_node *node) {
	char *key = mstring_create("Nodo%iTotal", node->id);
	dictionary_remove_and_destroy(config->properties, key, free);
	mstring_replace(&key, "Total", "Libre");
	dictionary_remove_and_destroy(config->properties, key, free);
	update_file();
}

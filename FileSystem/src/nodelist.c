#include "nodelist.h"
#include <mlist.h>
#include <stdlib.h>
#include <commons/config.h>
#include <path.h>
#include <file.h>
#include <system.h>
#include <mstring.h>
#include <serial.h>
#include <string.h>
#include <data.h>
#include "server.h"

static char *path = NULL;
static mlist_t *nodes = NULL;
static t_config *config = NULL;

static void init_config(void);
static void update_file(void);
static void add_node_to_file(t_node *node);
static void remove_node_from_file(t_node *node);
static void load_bitmap(t_node *node);
static t_node *create_node(const char *name, int blocks);
static void destroy_node(t_node *node);
static bool node_active(t_node *node);
static t_node *best_node_available(t_block *block);

// ========== Funciones públicas ==========

void nodelist_init() {
	if(nodes != NULL) return;
	path = mstring_create("%s/metadata/nodos.bin", system_userdir());
	nodes = mlist_create();
	init_config();

	char **snodes = config_get_array_value(config, "NODOS");
	if(snodes == NULL) return;

	for(char **pnode = snodes; *pnode != NULL; pnode++) {
		char *key = mstring_create("%sTotal", *pnode);
		int blocks = config_get_int_value(config, key);

		t_node *node = create_node(*pnode, blocks);
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

t_node * nodelist_get(int pos) {
	return mlist_get(nodes, pos);
}

bool nodelist_active(t_node *node) {
	return node_active(node);
}

t_node *nodelist_add(const char *name, int blocks) {
	t_node *node = nodelist_find(name);
	if(node == NULL) {
		node = create_node(name, blocks);
		mlist_append(nodes, node);
		add_node_to_file(node);
	}
	return node;
}

t_node *nodelist_find(const char *name) {
	if(mstring_isempty(name)) return NULL;
	bool finder(t_node *elem) {
		return mstring_equal(elem->name, name);
	}
	return mlist_find(nodes, finder);
}

void nodelist_addblock(t_block *block, void *data) {
	for(int i = 0; i < 2; i++) {
		t_node *node = best_node_available(block);
		if(node == NULL) continue;
		int blockno = bitmap_firstzero(node->bitmap);
		block->copies[i].blockno = blockno;
		block->copies[i].node = node->name;
		bitmap_set(node->bitmap, blockno);
		node->free_blocks--;
		add_node_to_file(node);
		t_nodeop *op = server_nodeop(NODE_SEND, blockno, serial_create(data, BLOCK_SIZE));
		thread_send(node->handler, op);
	}
}

void nodelist_remove(const char *name) {
	bool condition(t_node *elem) {
		return mstring_equal(elem->name, name);
	}
	t_node *node = mlist_remove(nodes, condition, NULL);
	if(node == NULL) return;

	remove_node_from_file(node);
	destroy_node(node);
}

void nodelist_clear() {
	mlist_clear(nodes, destroy_node);
	dictionary_clean_and_destroy_elements(config->properties, free);
	update_file();
}

void nodelist_print() {
	puts("Nombre\tActivo\tLibre\tTotal");
	void iterator(t_node *node) {
		printf("%s\t%s\t%i\t%i\n", node->name, node_active(node) ? "Sí" : "No",
				node->free_blocks, node->total_blocks);
	}
	mlist_traverse(nodes, iterator);
}

void nodelist_format(){
	for(int i = 0; i < nodelist_size(); i++){
		t_node* node = nodelist_get(i);
		if (!nodelist_active(node)){
			nodelist_remove(node->name);
		}else{
			bitmap_clear(node->bitmap);
		}
		node->free_blocks = node->total_blocks;
	}
	update_file();
}


void nodelist_term() {
	update_file();
	config_destroy(config);
	free(path);
	mlist_destroy(nodes, free);
}

// ========== Funciones privadas ==========

static void init_config() {
	if(config != NULL) return;
	path_mkfile(path);
	config = config_create(path);
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
		return mstring_duplicate(elem->name);
	}

	free(value);
	value = mlist_tostring(nodes, formatter);
	config_set_value(config, key, value);

	free(value);
	config_save(config);
}

static void add_node_to_file(t_node *node) {
	char *key = mstring_create("%sTotal", node->name);
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
	char *key = mstring_create("%sTotal", node->name);
	dictionary_remove_and_destroy(config->properties, key, free);
	mstring_replace(&key, "Total", "Libre");
	dictionary_remove_and_destroy(config->properties, key, free);
	update_file();
}

static void load_bitmap(t_node *node) {
	char *path = mstring_create("metadata/bitmaps/%s.dat", node->name);
	node->bitmap = bitmap_load(node->total_blocks, path);
	free(path);
}

static t_node *create_node(const char *name, int blocks) {
	t_node *node = malloc(sizeof(t_node));
	node->name = mstring_duplicate(name);
	node->total_blocks = blocks;
	node->free_blocks = blocks;
	load_bitmap(node);
	return node;
}

static void destroy_node(t_node *node) {
	free(node->name);
	free(node);
}

static bool node_active(t_node *node) {
	if(node == NULL || node->handler == NULL) return false;
	thread_send(node->handler, server_nodeop(NODE_PING, -1, NULL));
	bool active = (bool) thread_receive();
	return active;
}

static t_node *best_node_available(t_block *block) {
	int free_blocks = 0;
	t_node *node = NULL;
	char *prev = block->copies[0].node;
	void routine(t_node *elem) {
		if(!node_active(elem) || elem->free_blocks <= free_blocks
				|| (prev != NULL && mstring_equal(elem->name, prev))) return;
		free_blocks = elem->free_blocks;
		node = elem;
	}
	mlist_traverse(nodes, routine);
	return node;
}

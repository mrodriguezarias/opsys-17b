#include "filetable.h"

#include <commons/config.h>
#include <data.h>
#include <file.h>
#include <mlist.h>
#include <mstring.h>
#include <path.h>
#include <socket.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <system.h>
#include <thread.h>
#include <yfile.h>
#include <log.h>
#include <number.h>

#include "dirtree.h"
#include "nodelist.h"
#include "server.h"
#include "FileSystem.h"

static mlist_t *files = NULL;

static mlist_t *files_in_path(const char *path);
static void file_traverser(const char *path);
static void dir_traverser(t_directory *dir);
static void load_blocks(t_yfile *file, t_config *config);
static void save_blocks(t_yfile *file, t_config *config);
static t_yfile *create_file_from_config(t_config *config);
static void update_file(t_yfile *file);
static char *real_file_path(const char *path);
static bool add_blocks_from_file(t_yfile *yfile, const char *path);
static int copy_from_bin_file(t_file *source, char *buffer, t_yfile *target, t_node *onode);
static int copy_from_text_file(t_file *source, char *buffer, t_yfile *target, t_node *onode);
static int add_and_send_block(t_yfile *yfile, char *buffer, size_t size, t_node *onode);
static void print_block(t_block *block);
static void receive_blocks(t_yfile *file);
static void receive_block(t_block *block);
static t_block_copy *first_available_copy(t_block *block);
static bool available_copy(t_block *block);
static bool block_saved(t_block *block);

// ========== Funciones públicas ==========

void filetable_init() {
	if(files != NULL) return;
	files = mlist_create();
	dirtree_traverse(dir_traverser);
}

int filetable_size() {
	return mlist_length(files);
}

void filetable_add(t_yfile *file) {
	if(filetable_find(file->path) != NULL) return;
	mlist_append(files, file);
	path_mkfile(file->path);
	update_file(file);
}

t_yfile *filetable_find(const char *path) {
	char *rpath = real_file_path(path);
	if(mstring_isempty(rpath)) return NULL;

	bool cond(t_yfile *file) {
		return mstring_equal(file->path, rpath);
	}
	t_yfile *file = mlist_find(files, cond);
	free(rpath);
	return file;
}

bool filetable_contains(const char *path) {
	return filetable_find(path) != NULL;
}

void filetable_move(const char *path, const char *new_path) {
	t_yfile *file = filetable_find(path);
	if(file == NULL) return;
	char *npath = path_create(PTYPE_YAMA, new_path);
	if(!mstring_hassuffix(npath, path_name(file->path))) {
		mstring_format(&npath, "%s/%s", npath, path_name(file->path));
	}
	
	if(filetable_contains(npath)) {
		free(npath);
		return;
	}

	char *dpath = path_dir(npath);
	dirtree_add(dpath);
	free(dpath);

	char *rpath = real_file_path(npath);
	path_move(file->path, rpath);
	free(file->path);
	file->path = rpath;
}

void filetable_rename(const char *path, const char *new_name) {
	t_yfile *file = filetable_find(path);
	if(file == NULL) return;
	char *npath = path_dir(file->path);
	mstring_format(&npath, "%s/%s", npath, new_name);

	if(filetable_contains(npath)) {
		free(npath);
		return;
	}

	path_move(file->path, npath);
	free(file->path);
	file->path = npath;
}

void filetable_remove(const char *path) {
	t_yfile *file = filetable_find(path);
	if(file == NULL) return;

	bool cond(t_yfile *elem) {
		return mstring_equal(elem->path, file->path);
	}
	mlist_remove(files, cond, yfile_destroy);
	path_remove(file->path);
}

void filetable_clear() {
	void routine(t_yfile *elem) {
		filetable_remove(elem->path);
	}
	mlist_traverse(files, routine);
}

void filetable_print() {
	int size = filetable_size();
	if(size == 0) {
		printf("0 archivos.\n");
		return;
	}
	if(size == 1) printf("Un archivo:\n");
	else printf("%i archivos:\n", size);
	mlist_traverse(files, yfile_print);
}

void filetable_info(const char *path) {
	t_yfile *file = filetable_find(path);
	if(file == NULL) return;

	int nblk = mlist_length(file->blocks);
	printf("Archivo %s ", file->type == FTYPE_TXT ? "de texto" : "binario");

	if(nblk == 0) {
		printf("vacío.\n");
		return;
	}

	if(nblk == 1) printf("de un bloque ");
	else printf("de %i bloques ", nblk);

	char *size = mstring_bsize(file->size);
	printf("(%s):\n", size);
	free(size);

	printf("Bloque     Tamaño     Copia 1     Copia 2  \n");
	void block_printer(t_block *block) {
		char *size = mstring_bsize(block->size);
		printf("%6i  %9s  ", block->index, size);
		free(size);

		for(int i = 0; i < 2; i++) {
			char *node = block->copies[i].node;
			char *str = mstring_empty(NULL);
			if(node != NULL) {
				node = mstring_duplicate(node);
				mstring_crop(&node, 6);
				mstring_format(&str, "%s#%02i", node, block->copies[i].blockno);
				free(node);
			}
			printf("%10s  ", mstring_crop(&str, 10));
			free(str);
		}
		printf("\n");
	}
	mlist_traverse(file->blocks, block_printer);
}

size_t filetable_count(const char *path) {
	mlist_t *ctfiles = files_in_path(path);
	size_t count = mlist_length(ctfiles);
	mlist_destroy(ctfiles, NULL);
	return count;
}

void filetable_list(const char *path) {
	mlist_t *lsfiles = files_in_path(path);

	bool sorter(t_yfile *file1, t_yfile *file2) {
		return mstring_asc(path_name(file1->path), path_name(file2->path));
	}
	mlist_sort(lsfiles, sorter);

	void printer(t_yfile *file) {
		printf("%s\n", path_name(file->path));
	}
	mlist_traverse(lsfiles, printer);

	mlist_destroy(lsfiles, NULL);
}

void filetable_cpfrom(const char *path, const char *dir) {
	t_ftype type = path_istext(path) ? FTYPE_TXT : FTYPE_BIN;
	char *ypath = path_create(PTYPE_YAMA, dir, path_name(path));
	t_yfile *file = yfile_create(NULL, type);

	if(add_blocks_from_file(file, path)) {
		char *ydir = path_dir(ypath);
		dirtree_add(ydir);
		free(ydir);

		file->path = real_file_path(ypath);
		filetable_add(file);
	} else {
		yfile_destroy(file);
	}

	free(ypath);
}

void filetable_cpto(const char *path, const char *dir) {

}

void filetable_cat(const char *path) {
	t_yfile *file = filetable_find(path);
	if(file == NULL) return;
	receive_blocks(file);
}

bool filetable_stable(){
	bool available_block(t_yfile* file){
		return mlist_all(file->blocks, available_copy);
	}
	return (fs.formatted && mlist_all(files, available_block));
}

// ========== Funciones privadas ==========

static mlist_t *files_in_path(const char *path) {
	char *rpath = dirtree_rpath(path);
	if(rpath == NULL) return mlist_create();
	bool filter(t_yfile *file) {
		char *dir = path_dir(file->path);
		bool equal = path_equal(dir, rpath);
		free(dir);
		return equal;
	}
	mlist_t *list = mlist_filter(files, filter);
	free(rpath);
	return list;
}

static void file_traverser(const char *path) {
	t_config *config = config_create((char*)path);
	t_yfile *file = create_file_from_config(config);

	load_blocks(file, config);
	config_destroy(config);

	mlist_append(files, file);
}

static void dir_traverser(t_directory *dir) {
	char *dpath = dirtree_rpath(dir->name);
	path_files(dpath, file_traverser);
	free(dpath);
}

static void load_blocks(t_yfile *file, t_config *config) {
	char *key = mstring_empty(NULL);
	for(int blockno = 0; true; blockno++) {
		mstring_format(&key, "BLOQUE%iBYTES", blockno);
		char *sizestr = config_get_string_value(config, key);
		if(mstring_isempty(sizestr)) break;

		t_block *block = calloc(1, sizeof(t_block));
		block->index = blockno;
		block->size = mstring_toint(sizestr);

		for(int copyno = 0; copyno < 2; copyno++) {
			mstring_format(&key, "BLOQUE%iCOPIA%i", blockno, copyno);
			char **vals = config_get_array_value(config, key);
			block->copies[copyno].node = mstring_duplicate(vals[0]);
			block->copies[copyno].blockno = mstring_toint(vals[1]);
		}

		mlist_append(file->blocks, block);
	}
	free(key);
}

static void save_blocks(t_yfile *file, t_config *config) {
	char *key = mstring_empty(NULL);
	char *value = mstring_empty(NULL);
	void save_block(t_block *block) {
		mstring_format(&key, "BLOQUE%iBYTES", block->index);
		mstring_format(&value, "%i", block->size);
		config_set_value(config, key, value);
		for(int copyno = 0; copyno < 2; copyno++) {
			mstring_format(&key, "BLOQUE%iCOPIA%i", block->index, copyno);
			mstring_format(&value, "[%s, %d]", block->copies[copyno].node, block->copies[copyno].blockno);
			config_set_value(config, key, value);
		}
	}
	mlist_traverse(file->blocks, save_block);
	free(key);
	free(value);
}

static t_yfile *create_file_from_config(t_config *config) {
	t_yfile *file = malloc(sizeof(t_yfile));
	file->path = mstring_duplicate(config->path);
	file->size = config_get_long_value(config, "TAMANIO");
	file->type = mstring_equal(config_get_string_value(config, "TIPO"), "TEXTO") ? FTYPE_TXT : FTYPE_BIN;
	file->blocks = mlist_create();
	return file;
}

static void update_file(t_yfile *file) {
	if(!fs.formatted) return;

	t_config *config = config_create(file->path);
	config_set_value(config, "TIPO", file->type == FTYPE_TXT ? "TEXTO" : "BINARIO");
	char *sizestr = mstring_create("%i", file->size);
	config_set_value(config, "TAMANIO", sizestr);
	free(sizestr);

	save_blocks(file, config);
	config_save(config);
	config_destroy(config);
}

static char *real_file_path(const char *path) {
	if(mstring_hasprefix(path, system_userdir())) {
		return mstring_duplicate(path);
	}

	char *ypath = path_create(PTYPE_YAMA, path);
	char *pdir = path_dir(ypath);
	free(ypath);
	char *rpath = dirtree_rpath(pdir);
	free(pdir);
	if(rpath == NULL) return NULL;
	mstring_format(&rpath, "%s/%s", rpath, path_name(path));
	return rpath;
}


static bool add_blocks_from_file(t_yfile *yfile, const char *path) {
	char buffer[BLOCK_SIZE];
	t_file *source = file_open(path);
	int count, recount;

	if(yfile->type == FTYPE_TXT) {
		count = copy_from_text_file(source, buffer, NULL, NULL);
	} else {
		count = number_ceiling(file_size(source) * 1.0 / BLOCK_SIZE);
	}

	bool success = false;
	t_node *onode = nodelist_freestnode();

	if(onode == NULL) {
		fprintf(stderr, "Error: no hay nodos disponibles.\n");
		goto end;
	}

	if(onode->free_blocks < count) {
		fprintf(stderr, "Error: no hay suficiente espacio libre para guardar este archivo.\n");
		goto end;
	}

	if(yfile->type == FTYPE_TXT) {
		recount = copy_from_text_file(source, buffer, yfile, onode);
	} else {
		recount = copy_from_bin_file(source, buffer, yfile, onode);
	}

	success = count == recount;
	if(!success) fprintf(stderr, "Error: no se pudo guardar el archivo.\n");

	end:
	file_close(source);
	return success;
}

static int copy_from_bin_file(t_file *source, char *buffer, t_yfile *target, t_node *onode) {
	size_t size = 0;
	int count = 0;

	void block_handler(const void *block, size_t bsize) {
		printf("Copiando datos binarios\n");
		if(size + bsize > BLOCK_SIZE) {
			count += add_and_send_block(target, buffer, size, onode);
			size = 0;
		}
		memcpy(buffer + size, block, bsize);
		size += bsize;
	}
	file_btraverse(source, block_handler);
	count += add_and_send_block(target, buffer, size, onode);

	return count;
}

static int copy_from_text_file(t_file *source, char *buffer, t_yfile *target, t_node *onode) {
	size_t size = 0;
	int count = 0;

	void line_handler(const char *line) {
		printf("Copiando línea: %s\n", line);
		if(size + mstring_length(line) + 1 > BLOCK_SIZE) {
			count += add_and_send_block(target, buffer, size, onode);
			size = 0;
		}
		size += sprintf(buffer + size, "%s\n", line);
	}
	file_ltraverse(source, line_handler);
	count += add_and_send_block(target, buffer, size, onode);

	return count;
}

static int add_and_send_block(t_yfile *yfile, char *buffer, size_t size, t_node *onode) {
	if(size == 0) return 0;
	if(yfile != NULL) {
		t_block *block = calloc(1, sizeof(t_block));
		block->size = size;

		nodelist_addblock(block, buffer, onode);
		if(!block_saved(block)) return 0;

		yfile_addblock(yfile, block);
		print_block(block);
	}
	return 1;
}

static void print_block(t_block *block) {
	puts("---------- Info del bloque ----------");
	char *size = mstring_bsize(block->size);
	printf("Tamaño: %s\n", size);
	free(size);
	for(int i = 0; i < 2; i++) {
		printf("%s: ", i == 0 ? "Original" : "Copia");
		t_block_copy *copy = block->copies + i;
		if(copy == NULL || copy->node == NULL) printf("no guardada");
		else printf("en bloque %i de nodo %s", copy->blockno, copy->node);
		printf("\n");
	}
	puts("-------------------------------------");
}

static void receive_blocks(t_yfile *file) {
	server_set_current_file(file);
	puts("Recibiendo bloques...");
	mlist_traverse(file->blocks, receive_block);
	thread_suspend();
}

static void receive_block(t_block *block) {
	t_block_copy *copy = first_available_copy(block);
	t_node *node = nodelist_find(copy->node);
	t_nodeop *op = server_nodeop(NODE_RECV, copy->blockno, NULL);
	thread_send(node->handler, op);
}

static t_block_copy *first_available_copy(t_block *block) {
	t_block_copy *copy = NULL;
	t_node *node0 = nodelist_find(block->copies[0].node);
	t_node *node1 = nodelist_find(block->copies[1].node);
	while(copy == NULL) {
		if(nodelist_active(node0)) copy = block->copies;
		else if(nodelist_active(node1)) copy = block->copies + 1;
		else thread_sleep(500);
	}
	return copy;
}

static bool available_copy(t_block *block) {
	t_block_copy *copy = NULL;
	t_node *node0 = nodelist_find(block->copies[0].node);
	t_node *node1 = nodelist_find(block->copies[1].node);

	if(nodelist_active(node0)) copy = block->copies;
	else if(nodelist_active(node1)) copy = block->copies + 1;

	return (copy != NULL);
}

static bool block_saved(t_block *block) {
	return !mstring_isempty(block->copies[0].node);
}

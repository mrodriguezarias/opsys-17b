#include "filetable.h"
#include <mlist.h>
#include "dirtree.h"
#include <stdlib.h>
#include <path.h>
#include <commons/config.h>

#include <stdio.h>

static mlist_t *files = NULL;

static void file_traverser(const char *path);
static void dir_traverser(t_directory *dir);
static void load_blocks(t_yfile *file, t_config *config);
static void save_blocks(t_yfile *file, t_config *config);
static t_yfile *create_file_from_config(t_config *config);
static void update_file(t_yfile *file);

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
	char *rpath = yfile_path(path);
	if(mstring_isempty(rpath)) return NULL;

	bool cond(t_yfile *file) {
		return mstring_equal(file->path, rpath);
	}
	t_yfile *file = mlist_find(files, cond);
	free(rpath);
	return file;
}

void filetable_rename(const char *path, const char *new_name) {
	t_yfile *file = filetable_find(path);
	if(file == NULL) return;
	char *npath = path_dir(file->path);
	mstring_format(&npath, "%s/%s", npath, new_name);
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
	puts("Tabla de archivos yamafs");
	printf("Cantidad: %d\n", filetable_size());
	void routine(t_yfile *file) {
		yfile_print(file);
	}
	mlist_traverse(files, routine);
}

// ========== Funciones privadas ==========

static void file_traverser(const char *path) {
	t_config *config = config_create((char*)path);
	t_yfile *file = create_file_from_config(config);

	load_blocks(file, config);
	config_destroy(config);

	mlist_append(files, file);
}

static void dir_traverser(t_directory *dir) {
	char *dpath = dirtree_path(dir);
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
	t_config *config = config_create(file->path);
	config_set_value(config, "TIPO", file->type == FTYPE_TXT ? "TEXTO" : "BINARIO");
	char *sizestr = mstring_create("%i", file->size);
	config_set_value(config, "TAMANIO", sizestr);
	free(sizestr);

	save_blocks(file, config);
	config_save(config);
	config_destroy(config);
}

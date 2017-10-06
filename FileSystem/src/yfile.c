#include "yfile.h"

#include <data.h>
#include <mlist.h>
#include <mstring.h>
#include <path.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <system.h>

#include "dirtree.h"
#include "nodelist.h"

struct yfile {
	char *path;
	t_ftype type;
	mlist_t *blocks;
};

static const char *files_dir(void);
static char *real_path(const char *yama_path);

// ========== Funciones públicas ==========

t_yfile *yfile_create(const char *path, t_ftype type) {
	char *pdir = path_dir(path);
	dirtree_add(pdir);
	free(pdir);

	t_yfile *file = malloc(sizeof(t_yfile));
	file->path = real_path(path);
	file->type = type;
	file->blocks = mlist_create();
	return file;
}

void print_block(t_block *block) {
	printf("Block size: %d\n", block->size);
	printf("First copy: block #%d of node %s\n", block->copies[0].blockno, block->copies[0].node->name);
	printf("Second copy: block #%d of node %s\n", block->copies[1].blockno, block->copies[1].node->name);
}

void add_and_send_block(mlist_t *blocks, char *buffer, size_t size) {
	printf("Sending block:\n%s\n", buffer);
	t_block *block = calloc(1, sizeof(t_block));
	block->size = size;
	nodelist_addblock(block);
	print_block(block);
	mlist_append(blocks, block);
}

void add_blocks_from_text_file(mlist_t *blocks, const char *path) {
	t_file *file = file_open(path);

	char buffer[15];
	int size = 0;

	void line_handler(const char *line) {
		printf("copying line: %s\n", line);
		if(size + mstring_length(line) + 1 > 15) {
			add_and_send_block(blocks, buffer, BLOCK_SIZE);
			size = 0;
		}
		size += sprintf(buffer + size, "%s\n", line);
	}
	file_traverse(file, line_handler);
	add_and_send_block(blocks, buffer, size);
	file_close(file);
}

t_yfile *yfile_cpfrom(const char *path, const char *dir) {
	char *srcpath = path_create(PTYPE_USER, path);
	if(!path_isfile(srcpath)) return NULL;

	char *yama_path = path_create(PTYPE_YAMA, dir, path_name(srcpath));
	t_ftype type = path_istext(srcpath) ? FTYPE_TXT : FTYPE_BIN;

	t_yfile *file = yfile_create(yama_path, type);
	printf("source: %s\n", srcpath);
	printf("yama: %s\n", yama_path);
	printf("real: %s\n", file->path);

	if(type == FTYPE_TXT) {
//		add_blocks_from_text_file(file->blocks, srcpath);
	}

	return NULL;
}

void yfile_destroy(t_yfile *file) {
	free(file->path);
	free(file);
}

// ========== Funciones privadas ==========

static const char *files_dir() {
	static char *fdir = NULL;
	if(fdir == NULL) {
		fdir = mstring_create("%s/metadata/archivos", system_userdir());
	}
	return fdir;
}

static char *real_path(const char *yama_path) {
	char *pdir = path_dir(yama_path);
	t_directory *dir = dirtree_find(pdir);
	free(pdir);
	if(dir == NULL) return NULL;
	return mstring_create("%s/%d/%s", files_dir(), dir->index, path_name(yama_path));
}

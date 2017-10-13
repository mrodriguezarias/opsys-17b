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
#include "filetable.h"

// ========== Funciones públicas ==========

t_yfile *yfile_create(const char *path, t_ftype type) {
	char *pdir = path_dir(path);
	dirtree_add(pdir);
	free(pdir);

	t_yfile *file = malloc(sizeof(t_yfile));

	file->path = yfile_path(path);
	file->dir = path_dir(file->path);
	file->name = path_name(file->path);
	file->size = 0;
	file->type = type;
	file->blocks = mlist_create();
	return file;
}

char *yfile_path(const char *path) {
	char *ypath = path_create(PTYPE_YAMA, path);
	char *pdir = path_dir(ypath);
	free(ypath);
	char *rpath = dirtree_path(pdir);
	free(pdir);
	if(rpath == NULL) return NULL;
	mstring_format(&rpath, "%s/%s", rpath, path_name(path));
	return rpath;
}

void yfile_addblock(t_yfile *file, t_block *block) {
	block->index = mlist_length(file->blocks);
	mlist_append(file->blocks, block);
	file->size += block->size;
}

void print_block(t_block *block) {
	printf("Block size: %d\n", block->size);
	printf("First copy: block #%d of node %s\n", block->copies[0].blockno, block->copies[0].node);
	printf("Second copy: block #%d of node %s\n", block->copies[1].blockno, block->copies[1].node);
}

void add_and_send_block(t_yfile *yfile, char *buffer, size_t size) {
	printf("Sending block:\n%s\n", buffer);
	t_block *block = calloc(1, sizeof(t_block));
	block->size = size;
	nodelist_addblock(block, buffer);
	print_block(block);
	yfile_addblock(yfile, block);
}

void add_blocks_from_text_file(t_yfile *yfile, const char *path) {
	t_file *file = file_open(path);

	char buffer[15];
	int size = 0;

	void line_handler(const char *line) {
		printf("copying line: %s\n", line);
		if(size + mstring_length(line) + 1 > 15) {
			add_and_send_block(yfile, buffer, size);
			size = 0;
		}
		size += sprintf(buffer + size, "%s\n", line);
	}
	file_traverse(file, line_handler);
	add_and_send_block(yfile, buffer, size);
	file_close(file);
}

void yfile_cpfrom(const char *path, const char *dir) {
	char *srcpath = path_create(PTYPE_USER, path);
	if(!path_isfile(srcpath)) return;

	char *yama_path = path_create(PTYPE_YAMA, dir, path_name(srcpath));
	t_ftype type = path_istext(srcpath) ? FTYPE_TXT : FTYPE_BIN;

	t_yfile *file = yfile_create(yama_path, type);
	printf("source: %s\n", srcpath);
	printf("yama: %s\n", yama_path);
	printf("real: %s/%s\n", file->dir, file->name);

	if(type == FTYPE_TXT) {
		add_blocks_from_text_file(file, srcpath);
	}

	filetable_add(file);
}

void yfile_print(t_yfile *file) {
	printf("==== Archivo yamafs ====\n");
	if(file == NULL) {
		printf("(Archivo inexistente)\n");
		return;
	}
	printf("Ruta: %s/%s\n", file->dir, file->name);
	printf("Tipo: %s\n", file->type == FTYPE_TXT ? "TEXTO" : "BINARIO");
	printf("Tamaño: %d\n", file->size);
	printf("Bloques: %d\n", mlist_length(file->blocks));
	int blockno = 0;
	void routine(t_block *block) {
		printf("Bloque %d:\n", blockno++);
		printf(" Tamaño: %d\n", block->size);
		printf(" Copia 0: [%s, %d]\n", block->copies[0].node, block->copies[0].blockno);
		printf(" Copia 1: [%s, %d]\n", block->copies[1].node, block->copies[1].blockno);
	}
	mlist_traverse(file->blocks, routine);
}

void yfile_destroy(t_yfile *file) {
	free(file->dir);
	free(file->path);
	mlist_destroy(file->blocks, free);
	free(file);
}

// ========== Funciones privadas ==========

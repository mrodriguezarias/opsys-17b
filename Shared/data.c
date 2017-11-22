/**
 * Módulo para la manipulación del espacio de datos (data.bin).
 * Para ser usado únicamente por procesos Worker y DataNode.
 */

#include <path.h>
#include <fcntl.h>
#include <process.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <system.h>
#include <log.h>
#include <number.h>
#include <file.h>
#include "data.h"

static struct {
	size_t size;
	t_file *file;
	void *map;
} data;


void data_open(const char *path, size_t size) {
	if(!path_exists(path)) {
		path_truncate(path, size);
	} else {
		size = path_size(path);
	}
	data.size = size;
	data.file = file_open(path);
	data.map = file_map(data.file);
	char *bsize = mstring_bsize(data.size);
	log_print("%s mapeado a memoria (%s)", path_name(file_path(data.file)), bsize);
	free(bsize);
}

void data_set(int blockno, void *block) {
	void *pdata = data.map + blockno * BLOCK_SIZE;
	memcpy(pdata, block, BLOCK_SIZE);
	msync(pdata, BLOCK_SIZE, MS_SYNC);
}

void *data_get(int blockno) {
	return data.map + blockno * BLOCK_SIZE;
}

size_t data_size() {
	return data.size;
}

int data_blocks() {
	return data.size / BLOCK_SIZE;
}

void data_close() {
	file_unmap(data.file, data.map);
	file_close(data.file);
}

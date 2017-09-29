/**
 * Módulo para la manipulación del espacio de datos (data.bin).
 * Para ser usado únicamente por procesos Worker y DataNode.
 */

#include <file.h>
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
#include "data.h"

static struct {
	void *content;
	size_t size;
} data;


void data_open(const char *path, size_t size) {
	if(!file_exists(path)) {
		file_truncate(path, size);
	} else {
		size = file_size(path);
	}
	data.size = size;
	int mode, prot;
	if(process_current() == PROC_WORKER) {
		mode = O_RDONLY;
		prot = PROT_READ;
	} else {
		mode = O_RDWR;
		prot = PROT_READ | PROT_WRITE;
	}
	char *upath = system_upath(path);
	int fd = open(upath, mode);
	free(upath);
	data.content = mmap(NULL, size, prot, MAP_SHARED, fd, 0);
	if(data.content == MAP_FAILED) {
		fprintf(stderr, "mmap: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void data_set(int blockno, void *block) {
	memcpy(data.content + blockno * BLOCK_SIZE, block, BLOCK_SIZE);
}

void *data_get(int blockno) {
	return data.content + blockno * BLOCK_SIZE;
}

void *data_get_copy(int blockno) {
	void *block = malloc(BLOCK_SIZE);
	return memcpy(block, data.content + blockno * BLOCK_SIZE, BLOCK_SIZE);
}

size_t data_size() {
	return data.size;
}

void data_close() {
	munmap(data.content, data.size);
}

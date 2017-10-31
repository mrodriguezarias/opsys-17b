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
#include "data.h"

static struct {
	void *content;
	size_t size;
} data;


void data_open(const char *path, size_t size) {
	if(!path_exists(path)) {
		path_truncate(path, size);
	} else {
		size = path_size(path);
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
	char *bsize = mstring_bsize(data.size);
	log_print("data.bin mapeado a memoria (%s)", bsize);
	free(bsize);
}

void data_set(int blockno, void *block) {
	void *pdata = data.content + blockno * BLOCK_SIZE;
	memcpy(pdata, block, BLOCK_SIZE);
	msync(pdata, BLOCK_SIZE, MS_SYNC);
}

void *data_get(int blockno) {
	return data.content + blockno * BLOCK_SIZE;
}

size_t data_size() {
	return data.size;
}

int data_blocks() {
	return data.size / BLOCK_SIZE;
}

void data_close() {
	munmap(data.content, data.size);
}

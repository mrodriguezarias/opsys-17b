#include "bitmap.h"
#include <string.h>
#include <file.h>
#include <path.h>
#include <commons/bitarray.h>
#include <math.h>

#define byte_size(size) ceil((double) size / 8.0)

struct bitmap {
	t_bitarray *bitarray;
	t_file *file;
	size_t size;
};

void sync_file(t_bitmap *bitmap);

// ========== Funciones públicas ==========

t_bitmap *bitmap_create(size_t size) {
	size_t bytes = byte_size(size);
	t_bitmap *bitmap = malloc(sizeof(t_bitmap));
	char *bitarray = malloc(sizeof(char) * bytes);
	bitmap->bitarray = bitarray_create_with_mode(bitarray, bytes, LSB_FIRST);
	bitmap->file = NULL;
	bitmap->size = size;
	bitmap_clear(bitmap);
	return bitmap;
}

t_bitmap *bitmap_load(size_t size, const char *path) {
	t_bitmap *bitmap = bitmap_create(size);
	size = byte_size(size);
	bitmap->file = file_open(path);
	if(file_size(bitmap->file) >= size) {
		fread(bitmap->bitarray->bitarray, sizeof(char), size, file_pointer(bitmap->file));
	}
	return bitmap;
}

void bitmap_set(t_bitmap *bitmap, off_t bit) {
	bitarray_set_bit(bitmap->bitarray, bit);
	sync_file(bitmap);
}

void bitmap_unset(t_bitmap *bitmap, off_t bit) {
	bitarray_clean_bit(bitmap->bitarray, bit);
	sync_file(bitmap);
}

bool bitmap_test(t_bitmap *bitmap, off_t bit) {
	return bitarray_test_bit(bitmap->bitarray, bit);
}

off_t bitmap_firstzero(t_bitmap *bitmap) {
	for(off_t i = 0; i < bitmap->size; i++) {
		if(!bitmap_test(bitmap, i)) return i;
	}
	return -1;
}

off_t bitmap_firstone(t_bitmap *bitmap) {
	for(off_t i = 0; i < bitmap->size; i++) {
		if(bitmap_test(bitmap, i)) return i;
	}
	return -1;
}

void bitmap_clear(t_bitmap *bitmap) {
	memset(bitmap->bitarray->bitarray, 0, bitmap->bitarray->size);
	sync_file(bitmap);
}

void bitmap_destroy(t_bitmap *bitmap) {
	file_close(bitmap->file);
	free(bitmap->bitarray->bitarray);
	bitarray_destroy(bitmap->bitarray);
	free(bitmap);
}

// ========== Funciones privadas ==========

void sync_file(t_bitmap *bitmap) {
	if(bitmap->file == NULL) return;
	rewind(file_pointer(bitmap->file));
	fwrite(bitmap->bitarray->bitarray, sizeof(char), bitmap->bitarray->size, file_pointer(bitmap->file));
}


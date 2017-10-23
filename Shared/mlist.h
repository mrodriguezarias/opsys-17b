#ifndef mlist_h
#define mlist_h

#include <stddef.h>
#include <stdbool.h>

typedef struct mlist mlist_t;

mlist_t *mlist_create(void);

bool mlist_empty(mlist_t *list);

int mlist_length(mlist_t *list);

void *mlist_first(mlist_t *list);

void *mlist_last(mlist_t *list);

mlist_t *mlist_copy(mlist_t *list);

mlist_t *mlist_clone(mlist_t *list, size_t elem_size);

void mlist_append(mlist_t *list, void *element);

void mlist_insert(mlist_t *list, int index, void *element);

void mlist_extend(mlist_t *list, mlist_t *other);

void *mlist_remove(mlist_t *list, void *condition, void *destroyer);

void *mlist_pop(mlist_t *list, int index);

void *mlist_get(mlist_t *list, int index);

int mlist_index(mlist_t *list, void *condition);

void *mlist_find(mlist_t *list, void *condition);

bool mlist_contains(mlist_t *list, void *element);

void *mlist_random(mlist_t *list);

void *mlist_replace(mlist_t *list, int index, void *element);

void mlist_traverse(mlist_t *list, void *routine);

mlist_t *mlist_reverse(mlist_t *list);

void mlist_sort(mlist_t *list, void *comparator);

void mlist_rsort(mlist_t *list, void *comparator);

int mlist_count(mlist_t *list, void *condition);

mlist_t *mlist_filter(mlist_t *list, void *filter);

mlist_t *mlist_map(mlist_t *list, void *mapper);

int mlist_reduce(mlist_t *list, void *adder);

bool mlist_any(mlist_t *list, void *condition);

bool mlist_all(mlist_t *list, void *condition);

void mlist_clear(mlist_t *list, void *destroyer);

mlist_t *mlist_fromstring(const char *format, ...);

char *mlist_tostring(mlist_t *list, void *formatter);

void mlist_print(mlist_t *list);

void mlist_destroy(mlist_t *list, void *destroyer);

#endif /* mlist_h */

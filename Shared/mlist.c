/**
 * Como la de las commons, pero mejor (por eso la eme :3)
 * - Thread-safe
 * - Algoritmos más eficientes
 * - Merge sort
 * - Muchas más funciones
 */

#include "mlist.h"
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "thread.h"

#define is_empty(list) (list->length == 0)
#define get_elem(node) (node != NULL ? node->elem : NULL)
#define mutex_lock(list) pthread_mutex_lock(&list->mutex)
#define mutex_unlock(list) pthread_mutex_unlock(&list->mutex)

typedef struct node {
	struct node *prev;
	struct node *next;
	void *elem;
} node_t;

struct mlist {
	node_t *head;
	node_t *tail;
	int length;
	pthread_mutex_t mutex;
};

static node_t *create_node(void *element);
static void insert_before(mlist_t *list, node_t *base, node_t *new);
static void insert_after(mlist_t *list, node_t *base, node_t *new);
static node_t *get_node(mlist_t *list, int index);
static void traverse_nodes(mlist_t *list, void node_fn(node_t*), void elem_fn(void*),
		bool cond(void*), bool reversed);
static void *remove_node(mlist_t *list, node_t *node);
static void split_nodes(node_t *source, node_t **front, node_t **back);
static node_t *merge_nodes(node_t *a, node_t *b, bool(*cmp)(void*,void*), bool fwd);
static void sort_nodes(node_t **nodes, void *cmp, bool fwd);


// ========== Funciones públicas ==========

mlist_t *mlist_create() {
	mlist_t *list = malloc(sizeof(mlist_t));
	list->head = NULL;
	list->tail = NULL;
	list->length = 0;
	pthread_mutex_init(&list->mutex, NULL);
	return list;
}

bool mlist_empty(mlist_t *list) {
	return is_empty(list);
}

int mlist_length(mlist_t *list) {
	return list->length;
}

void *mlist_first(mlist_t *list) {
	return !is_empty(list) ? list->head->elem : NULL;
}

void *mlist_last(mlist_t *list) {
	return !is_empty(list) ? list->tail->elem : NULL;
}

mlist_t *mlist_copy(mlist_t *list) {
	mlist_t *copy = mlist_create();
	void routine(void *element) {
		mlist_append(copy, element);
	}
	mlist_traverse(list, routine);
	return copy;
}

mlist_t *mlist_clone(mlist_t *list, size_t elem_size) {
	mlist_t *clone = mlist_create();
	void routine(void *element) {
		void *new_elem = malloc(elem_size);
		memmove(new_elem, element, elem_size);
		mlist_append(clone, new_elem);
	}
	mlist_traverse(list, routine);
	return clone;
}

void mlist_append(mlist_t *list, void *element) {
	node_t *node = create_node(element);
	insert_after(list, list->tail, node);
}

void mlist_insert(mlist_t *list, int index, void *element) {
	if(index < 0 || index > list->length) return;
	node_t *new = create_node(element);
	if(index < list->length) {
		insert_before(list, index == 0 ? list->head : get_node(list, index), new);
	} else {
		insert_after(list, list->tail, new);
	}
}

void mlist_extend(mlist_t *list, mlist_t *other) {
	void routine(void *element) {
		mlist_append(list, element);
	}
	mlist_traverse(other, routine);
}

void *mlist_remove(mlist_t *list, void *condition, void *destroyer) {
	bool (*cond)(void*) = condition;
	void (*elem_fn)(void*) = destroyer;

	void *elem = NULL;
	void node_fn(node_t *node) {
		elem = remove_node(list, node);
	}
	traverse_nodes(list, node_fn, elem_fn, cond, false);
	return elem;
}

void *mlist_pop(mlist_t *list, int index) {
	node_t *node = get_node(list, index);
	if(node == NULL) return NULL;
	return remove_node(list, node);
}

void *mlist_get(mlist_t *list, int index) {
	node_t *node = get_node(list, index);
	return get_elem(node);
}

int mlist_index(mlist_t *list, void *condition) {
	bool (*cond)(void*) = condition;
	int index = 0;
	for(node_t *node = list->head; node != NULL && !cond(node->elem); node = node->next) {
		index++;
	}
	return index < list->length ? index : -1;
}

void *mlist_find(mlist_t *list, void *condition) {
	bool (*cond)(void*) = condition;
	node_t *node;
	for(node = list->head; node != NULL && !cond(node->elem); node = node->next);
	return get_elem(node);
}

bool mlist_contains(mlist_t *list, void *element) {
	bool condition(void *current) {
		return current == element;
	}
	int index = mlist_index(list, condition);
	return index != -1;
}

void *mlist_replace(mlist_t *list, int index, void *element) {
	node_t *node = get_node(list, index);
	if(node == NULL) return NULL;
	void *prev_elem = node->elem;
	mutex_lock(list);
	node->elem = element;
	mutex_unlock(list);
	return prev_elem;
}

void mlist_traverse(mlist_t *list, void *routine) {
	traverse_nodes(list, NULL, routine, NULL, false);
}

mlist_t *mlist_reverse(mlist_t *list) {
	mlist_t *reversed = mlist_create();
	void elem_fn(void *element) {
		mlist_append(reversed, element);
	}
	traverse_nodes(list, NULL, elem_fn, NULL, true);
	return reversed;
}

void mlist_sort(mlist_t *list, void *comparator) {
	sort_nodes(&list->head, comparator, true);
}

void mlist_rsort(mlist_t *list, void *comparator) {
	sort_nodes(&list->head, comparator, false);
}

int mlist_count(mlist_t *list, void *condition) {
	int count = 0;
	void node_fn(node_t *node) {
		if(node != NULL) count++;
	}
	traverse_nodes(list, node_fn, NULL, condition, false);
	return count;
}

mlist_t *mlist_filter(mlist_t *list, void *filter) {
	bool (*condition)(void*) = filter;
	mlist_t *new_list = mlist_create();
	void routine(void *element) {
		if(condition(element)) mlist_append(new_list, element);
	}
	mlist_traverse(list, routine);
	return new_list;
}

mlist_t *mlist_map(mlist_t *list, void *mapper) {
	void *(*map)(void*) = mapper;
	mlist_t *new_list = mlist_create();
	void routine(void *element) {
		mlist_append(new_list, map(element));
	}
	mlist_traverse(list, routine);
	return new_list;
}

bool mlist_any(mlist_t *list, void *condition) {
	return mlist_find(list, condition) != NULL;
}

bool mlist_all(mlist_t *list, void *condition) {
	bool (*positive)(void*) = condition;
	bool negative(void *element) {
		return !positive(element);
	}
	return !mlist_any(list, negative);
}

void mlist_clear(mlist_t *list, void *destroyer) {
	void (*elem_fn)(void*) = destroyer;

	mutex_lock(list);
	traverse_nodes(list, (void(*)(node_t*)) free, elem_fn, NULL, false);
	mutex_unlock(list);

	list->head = list->tail = NULL;
	list->length = 0;
}

void mlist_destroy(mlist_t *list, void *destroyer) {
	mlist_clear(list, destroyer);
	thread_mutex_destroy(&list->mutex);
	free(list);
}

// ========== Funciones privadas ==========

static node_t *create_node(void *element) {
	node_t *node = malloc(sizeof(node_t));
	node->prev = NULL;
	node->next = NULL;
	node->elem = element;
	return node;
}

static void insert_before(mlist_t *list, node_t *base, node_t *new) {
	mutex_lock(list);
	if(is_empty(list)) {
		list->head = list->tail = new;
	} else {
		new->prev = base->prev;
		new->next = base;
		base->prev = new;
		if(new->prev != NULL) {
			new->prev->next = new;
		} else {
			list->head = new;
		}
	}
	list->length++;
	mutex_unlock(list);
}

static void insert_after(mlist_t *list, node_t *base, node_t *new) {
	mutex_lock(list);
	if(is_empty(list)) {
		list->head = list->tail = new;
	} else {
		new->next = base->next;
		new->prev = base;
		base->next = new;
		if(new->next != NULL) {
			new->next->prev = new;
		} else {
			list->tail = new;
		}
	}
	list->length++;
	mutex_unlock(list);
}

static node_t *get_node(mlist_t *list, int index) {
	if(index < 0 || index >= list->length) {
		return NULL;
	}
	node_t *node;
	if(index <= list->length / 2) {
		node = list->head;
		while(index-- > 0) {
			node = node->next;
		}
	} else {
		node = list->tail;
		while(++index < list->length) {
			node = node->prev;
		}
	}
	return node;
}

static void traverse_nodes(mlist_t *list, void node_fn(node_t*), void elem_fn(void*),
		bool cond(void*), bool reversed) {
	node_t *next, *node = reversed ? list->tail : list->head;
	while(node != NULL) {
		next = reversed ? node->prev : node->next;
		if(cond == NULL || cond(node->elem)) {
			if(elem_fn != NULL) elem_fn(node->elem);
			if(node_fn != NULL) node_fn(node);
		}
		node = next;
	}
}

static void *remove_node(mlist_t *list, node_t *node) {
	mutex_lock(list);
	if(node->prev != NULL) node->prev->next = node->next;
	if(node->next != NULL) node->next->prev = node->prev;
	if(node == list->head) list->head = node->next;
	if(node == list->tail) list->tail = node->prev;
	list->length--;
	mutex_unlock(list);
	void *element = node->elem;
	free(node);
	return element;
}

static void split_nodes(node_t *source, node_t **front, node_t **back) {
	node_t *slow, *fast;

	if(source == NULL || source->next==NULL) {
		*front = source;
		*back = NULL;
		return;
	}

	slow = source;
	fast = source->next;

	while(fast != NULL) {
		fast = fast->next;
		if(fast != NULL) {
			slow = slow->next;
			fast = fast->next;
		}
	}

	*front = source;
	*back = slow->next;
	slow->next = NULL;
}

static node_t *merge_nodes(node_t *a, node_t *b, bool(*cmp)(void*,void*), bool fwd) {
	node_t *result = NULL;

	if(a == NULL) return b;
	else if (b == NULL) return a;

	if((fwd && cmp(a->elem, b->elem)) || (!fwd && !cmp(a->elem, b->elem))) {
		result = a;
		result->next = merge_nodes(a->next, b, cmp, fwd);
	} else {
		result = b;
		result->next = merge_nodes(a, b->next, cmp, fwd);
	}

	return result;
}

static void sort_nodes(node_t **nodes, void *cmp, bool fwd) {
	node_t *head = *nodes;
	if(head == NULL || head->next == NULL) return;

	node_t *a, *b;
	split_nodes(head, &a, &b);

	sort_nodes(&a, cmp, fwd);
	sort_nodes(&b, cmp, fwd);

	*nodes = merge_nodes(a, b, cmp, fwd);
}

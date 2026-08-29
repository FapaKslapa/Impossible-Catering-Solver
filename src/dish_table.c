#include "dish_table.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *name;
    int index;
} DishSlot;

struct DishTable {
    DishSlot *slots;
    size_t capacity;
    int count;
};

#define DISH_TABLE_INITIAL_CAPACITY 16

static unsigned long fnv1a_hash(const char *s) {
    unsigned long hash = 2166136261u;
    while (*s != '\0') {
        hash ^= (unsigned char) *s;
        hash *= 16777619u;
        s++;
    }
    return hash;
}

static void dish_table_insert_slot(DishSlot *slots, size_t capacity, char *name, int index) {
    size_t position = fnv1a_hash(name) % capacity;
    while (slots[position].name != NULL) {
        position = (position + 1) % capacity;
    }
    slots[position].name = name;
    slots[position].index = index;
}

static void dish_table_grow(DishTable *table) {
    size_t new_capacity = table->capacity * 2;
    DishSlot *new_slots = calloc(new_capacity, sizeof(DishSlot));
    for (size_t i = 0; i < table->capacity; i++) {
        if (table->slots[i].name != NULL) {
            dish_table_insert_slot(new_slots, new_capacity, table->slots[i].name, table->slots[i].index);
        }
    }
    free(table->slots);
    table->slots = new_slots;
    table->capacity = new_capacity;
}

DishTable *dish_table_create(void) {
    DishTable *table = malloc(sizeof(DishTable));
    table->capacity = DISH_TABLE_INITIAL_CAPACITY;
    table->slots = calloc(table->capacity, sizeof(DishSlot));
    table->count = 0;
    return table;
}

void dish_table_destroy(DishTable *table) {
    for (size_t i = 0; i < table->capacity; i++) {
        free(table->slots[i].name);
    }
    free(table->slots);
    free(table);
}

bool dish_table_try_get(const DishTable *table, const char *name, int *out_index) {
    size_t position = fnv1a_hash(name) % table->capacity;
    while (table->slots[position].name != NULL) {
        if (strcmp(table->slots[position].name, name) == 0) {
            *out_index = table->slots[position].index;
            return true;
        }
        position = (position + 1) % table->capacity;
    }
    return false;
}

int dish_table_get_or_add(DishTable *table, const char *name) {
    int existing_index;
    if (dish_table_try_get(table, name, &existing_index)) {
        return existing_index;
    }
    if ((size_t) (table->count + 1) * 10 >= table->capacity * 7) {
        dish_table_grow(table);
    }
    int new_index = table->count;
    table->count++;
    dish_table_insert_slot(table->slots, table->capacity, strdup(name), new_index);
    return new_index;
}

int dish_table_count(const DishTable *table) {
    return table->count;
}

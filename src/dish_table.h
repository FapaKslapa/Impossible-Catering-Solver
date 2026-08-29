#ifndef DISH_TABLE_H
#define DISH_TABLE_H

#include <stddef.h>
#include <stdbool.h>

typedef struct DishTable DishTable;

DishTable *dish_table_create(void);
void dish_table_destroy(DishTable *table);
int dish_table_get_or_add(DishTable *table, const char *name);
bool dish_table_try_get(const DishTable *table, const char *name, int *out_index);
int dish_table_count(const DishTable *table);

#endif

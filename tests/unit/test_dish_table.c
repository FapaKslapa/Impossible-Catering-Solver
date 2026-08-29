#include "dish_table.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    DishTable *table = dish_table_create();

    int pizza_index = dish_table_get_or_add(table, "pizza");
    int sushi_index = dish_table_get_or_add(table, "sushi");
    int pizza_again = dish_table_get_or_add(table, "pizza");

    assert(pizza_index == pizza_again);
    assert(pizza_index != sushi_index);
    assert(dish_table_count(table) == 2);

    int found_index;
    assert(dish_table_try_get(table, "sushi", &found_index));
    assert(found_index == sushi_index);

    assert(!dish_table_try_get(table, "risotto", &found_index));

    for (int i = 0; i < 200; i++) {
        char name[32];
        snprintf(name, sizeof(name), "dish_%d", i);
        dish_table_get_or_add(table, name);
    }
    assert(dish_table_count(table) == 202);
    assert(dish_table_try_get(table, "dish_150", &found_index));

    dish_table_destroy(table);

    printf("test_dish_table: OK\n");
    return 0;
}

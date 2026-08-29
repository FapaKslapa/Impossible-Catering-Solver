#include "sat_restart.h"
#include <assert.h>
#include <stdio.h>

int main(void) {
    long long expected[] = {1, 1, 2, 1, 1, 2, 4, 1, 1, 2, 1, 1, 2, 4, 8};
    int count = 15;
    for (int i = 0; i < count; i++) {
        long long actual = sat_luby(i + 1);
        assert(actual == expected[i]);
    }
    printf("test_sat_restart: OK\n");
    return 0;
}

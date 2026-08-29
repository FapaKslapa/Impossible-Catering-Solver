#include "sat_restart.h"

long long sat_luby(long long index) {
    long long position = index - 1;
    long long size = 1;
    long long sequence = 0;
    while (size < position + 1) {
        sequence++;
        size = 2 * size + 1;
    }
    while (size - 1 != position) {
        size = (size - 1) / 2;
        sequence--;
        position = position % size;
    }
    return (long long) 1 << sequence;
}

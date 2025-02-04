#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include "crazylist.h"

crazycons_t *enclosing_struct(uint64_t *car) {
    return (crazycons_t *) ((void *) car - offsetof(crazycons_t, car));
}

uint64_t *cons(uint64_t car, uint64_t *cdr) {
    crazycons_t *cons = (crazycons_t *) malloc(sizeof(crazycons_t));
    assert(cons);
    cons->car = car;
    cons->cdr = cdr;
    assert(cons);
    return (uint64_t *) &cons->car;
}

uint64_t first(uint64_t *list) {
    assert(0); // FIXME
}

uint64_t *rest(uint64_t *list) {
    assert(0); // FIXME
}

uint64_t *find(uint64_t *list, uint64_t query) {
    assert(0); // FIXME
}

uint64_t *insert_sorted(uint64_t *list, uint64_t n) {
    assert(0); // FIXME
}

uint64_t *reverse(uint64_t *list) {
    assert(0); // FIXME
}

void print_list(uint64_t *list) {
    assert(0); // FIXME
}

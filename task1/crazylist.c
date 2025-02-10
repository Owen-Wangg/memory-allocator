#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include "crazylist.h"
#include <stdio.h>  // For printf

crazycons_t *enclosing_struct(uint64_t *car) {
    // return (crazycons_t *) ((void *) car - offsetof(crazycons_t, car));

    /*
    (crazycons_t *)0 creates a hypothetical structure at address 0
    &((crazycons_t *)0)->car gets the address of the car member relative to this base address 0
    This gives the same offset that offsetof would provide
    Cast to char * to do pointer arithmetic at the byte level
    Finally cast back to crazycons_t * to return the structure pointer    
    */
    return (crazycons_t *)((char *)car - (char *)&((crazycons_t *)0)->car);
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
    return *list;   //dereference the pointer to get the value
}

uint64_t *rest(uint64_t *list) {
    crazycons_t *cell = enclosing_struct(list); //returns: the crazycons_t cell that encloses the given car
    return cell->cdr;   //return pointer to next cell
}

uint64_t *find(uint64_t *list, uint64_t query) {
    while(list != NULL) {
        if(first(list) == query){
            return list;
        }
        list = rest(list);  //move to next cell
    }
    return NULL;
}

uint64_t *insert_sorted(uint64_t *list, uint64_t n) {
    if(first(list) >= n || list == NULL){   //inserting at the beginning 
        return cons(n,list);
    }

    uint64_t *current = list;
    while (rest(current) != NULL && first(rest(current)) < n) { //finding correct insertion position
        current = rest(current);
    }

    crazycons_t *cell = enclosing_struct(current);  //returns: the crazycons_t cell that encloses the given car 
    cell->cdr = cons(n, cell->cdr); //create new cell with n and update the cdr
    return list;
}

uint64_t *reverse(uint64_t *list) {
    uint64_t *prev = NULL;
    uint64_t *current = list;
    uint64_t *next = NULL;

    while (current != NULL) {
        next = rest(current);
        crazycons_t *cell = enclosing_struct(current);
        cell->cdr = prev;
        prev = current;
        current = next;
    }
    return prev;    //prev points to new head of reversed list
}

void print_list(uint64_t *list) {
    while (list != NULL) {
        printf("%" PRIu64 " ", first(list));    //PRIu64 ensures the correct format specifier for printing uint64_t
        list = rest(list);
    }
    printf("\n");
}

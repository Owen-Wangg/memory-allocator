#ifndef __CRAZYLIST_H__
#define __CRAZYLIST_H__

#include <stdint.h>

typedef struct {
    uint64_t *cdr; // pointer to the next cons cell, or NULL if the list ends at this cell
    uint64_t car;  // <-- cons pointers are (uint64_t *) and point to here
} crazycons_t;

// returns: the crazycons_t cell that encloses the given car
// side effects: none
crazycons_t *enclosing_struct(uint64_t *car);

// returns: a cons cell with the given car and cdr
// side effects: heap memory allocated for the new cell
uint64_t *cons(uint64_t car, uint64_t *cdr);

// returns: the car of the cons cell
// side effects: none
uint64_t first(uint64_t *list);

// returns: the cdr of the cons cell
// side effects: none
uint64_t *rest(uint64_t *list);

// returns: the cons containing the first occurrence of query in the list if found, otherwise NULL
// side effects: none
uint64_t *find(uint64_t *list, uint64_t query);

// description: if list is sorted in non-decreasing order, insert n into list in-place
//              so that it remains sorted
// returns: the head of the resulting list
// side effects:
// - heap memory allocated for one cell
// - up to 1 cell modified to reflect in-place insertion
uint64_t *insert_sorted(uint64_t *list, uint64_t n);

// description: reverses the order of the list elements in-place, with at a constant
//              space overhead of at most four pointers;
// returns: the head of the resulting list
// side effects:
// - no heap memory allocated
// - at most N cells modified, where N is the length of the list
uint64_t *reverse(uint64_t *list);

#endif // __CRAZYLIST_H__

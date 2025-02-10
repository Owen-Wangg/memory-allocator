#include <stdio.h>
#include <stdlib.h>
#include "crazylist.h"

void print_list(uint64_t *list);

int main() {
    // Test cons and print_list
    uint64_t *list = cons(100, NULL);
    list = cons(30, list);
    list = cons(10, list);
    list = cons(5, list);
    printf("Initial list: ");
    print_list(list); // Expected output: 5 10 30 100

    // Test insert_sorted
    list = insert_sorted(list, 50);
    printf("After inserting 50: ");
    print_list(list); // Expected output: 5 10 30 50 100

    // Test find and modify the list
    uint64_t *found = find(list, 10);
    if (found != NULL) {
        *found = 20;
    }
    printf("After modifying 10 to 20: ");
    print_list(list); // Expected output: 5 20 30 50 100

    // Test reverse
    list = reverse(list);
    printf("After reversing the list: ");
    print_list(list); // Expected output: 100 50 30 20 5

    // Test enclosing_struct
    crazycons_t *cell = enclosing_struct(found);
    if (cell != NULL) {
        printf("Enclosing struct of 20: car=%lu, cdr=%p\n", cell->car, (void*)cell->cdr);
    }

    // Test first and rest
    printf("First element: %lu\n", first(list));
    printf("Rest of the list: ");
    print_list(rest(list));

    return 0;
}
#pragma once

#include <stdbool.h>
#include <stdlib.h>

// ADT base structure
typedef struct _entry
{
    struct _entry* next;
    // Type specific data follows (dynamic length)
} entry_t;

//------------------------------------------------------------------------------------------------------------------------
 /*** Common ***/

typedef void
adt_common_destroy_fn(entry_t* entry);

/*** Queue ***/

void
adt_queue_push_back(entry_t** head, entry_t* entry);

entry_t*
adt_queue_peek_front(const entry_t* head); // just look at entry from queue

entry_t*
adt_queue_pop_front(entry_t** head); // remove entry from queue

void
adt_queue_destroy(entry_t** head, adt_common_destroy_fn destroy_fn);


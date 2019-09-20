
#include "adt.h"

void
adt_queue_push_back(entry_t** head, entry_t* entry)
{
    if (!head || !entry) {
        return;
    }

    entry_t* e;
    for (e = *head; e && e->next; e = e->next) {
        // fast forward
    }

    if (!e) {
        *head = entry;
    } else {
        e->next = entry;
    }
}

//------------------------------------------------------------------------------------------------------------------------

entry_t*
adt_queue_pop_front(entry_t** head)
{
    if (!head || !(*head)) {
        return NULL;
    }

    entry_t* e = *head;
    *head = e->next;
    return e;
}

//------------------------------------------------------------------------------------------------------------------------

entry_t*
adt_queue_peek_front(const entry_t* head)
{
    if (!head) {
        return NULL;
    }

    return (entry_t*)head;
}

//------------------------------------------------------------------------------------------------------------------------

void
adt_queue_destroy(entry_t** head, adt_common_destroy_fn destroy_fn)
{
    entry_t* e;
    entry_t* next;

    for (e = *head; e; e = next) {
        next = e->next;
        destroy_fn(e);
    }

    *head = NULL;
}

//------------------------------------------------------------------------------------------------------------------------


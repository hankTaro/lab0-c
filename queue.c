#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *new = malloc(sizeof(struct list_head));
    if (!new)
        return NULL;

    INIT_LIST_HEAD(new);

    return new;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (list_empty(l))
        return;
    element_t *temp, *it;
    list_for_each_entry_safe (it, temp, l, list) {
        q_release_element(it);
    }
    free(l);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new = malloc(sizeof(element_t));
    if (!new)
        return false;

    new->value = strdup(s);
    if (!new->value)  // if allocate failed
    {
        free(new);
        return false;
    }
    list_add(&new->list, head);
    return true;
}


/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new = malloc(sizeof(element_t));
    if (!new)
        return false;

    new->value = strdup(s);
    if (!new->value)  // if allocate failed
    {
        free(new);
        return false;
    }
    list_add_tail(&new->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *remove_loc = list_first_entry(head, element_t, list);
    list_del(&remove_loc->list);
    if (sp) {
        strncpy(sp, remove_loc->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return remove_loc;
}


/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *remove_loc = list_last_entry(head, element_t, list);
    list_del(&remove_loc->list);
    if (sp) {
        strncpy(sp, remove_loc->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return remove_loc;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    int count = 0;
    struct list_head *it;
    list_for_each (it, head)
        count++;
    return count;
}


/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;
    struct list_head *l = head->next;
    struct list_head *r = head->prev;
    while (r != l && r->next != l) {
        r = r->next;
        l = l->prev;
    }
    list_del(r);
    element_t *del = list_entry(r, element_t, list);
    q_release_element(del);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;
    struct list_head *start = head;
    element_t *it, *safe;
    LIST_HEAD(del);
    list_for_each_entry_safe (it, safe, head, list) {
        if (&safe->list != head && strcmp(safe->value, it->value) == 0)
            continue;
        /* Detect duplicated elements */
        if (it->list.prev != start) {
            LIST_HEAD(tmp);
            list_cut_position(&tmp, start, &it->list);
            list_splice(&tmp, &del);
        }
        start = safe->list.prev;
    }
    /* free del */
    list_for_each_entry_safe (it, safe, &del, list)
        q_release_element(it);
    return true;
}
/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    struct list_head *cur = head, *prev = head->prev, *next = NULL;
    while (next != head) {
        next = cur->next;
        cur->next = prev;
        cur->prev = next;
        prev = cur;
        cur = next;
    }
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head))
        return;
    LIST_HEAD(make_rev);
    int count = 0;
    struct list_head *it, *safe, *start = head;
    list_for_each_safe (it, safe, head) {
        count++;
        if (count == k) {
            count = 0;
            list_cut_position(&make_rev, start, it);
            q_reverse(&make_rev);
            list_splice_init(&make_rev, start);
            start = safe->prev;
        }
    }
}

struct list_head *merge_two_lists(struct list_head *L1, struct list_head *L2)
{
    struct list_head *head = NULL, **ptr = &head, **node;

    for (node = NULL; L1 && L2; *node = (*node)->next) {
        element_t *E1 = list_entry(L1, element_t, list);
        element_t *E2 = list_entry(L2, element_t, list);
        node = strcmp(E1->value, E2->value) ? &L1 : &L2;
        *ptr = *node;
        ptr = &(*ptr)->next;
    }
    *ptr = (struct list_head *) ((u_int64_t) L1 | (u_int64_t) L2);
    return head;
}

struct list_head *mergesort(struct list_head *head)
{
    if (!head || !head->next)
        return head;
    struct list_head *fast = head, *slow = head;
    while (fast && fast->next) {
        fast = fast->next->next;
        slow = slow->next;
    }
    fast = slow;
    slow->prev->next = NULL;
    struct list_head *L1 = mergesort(head);
    struct list_head *L2 = mergesort(fast);
    return merge_two_lists(L1, L2);
}

/* Sort elements of queue in ascending order */
void q_sort(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    head->next = mergesort(head->next);
    struct list_head *current = head, *next = head->next;
    while (next) {
        next->prev = current;
        current = next;
        next = next->next;
    }
    current->next = head;
    head->prev = current;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}

/* Merge all the queues into one sorted queue, which is in ascending order */
int q_merge(struct list_head *head)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}

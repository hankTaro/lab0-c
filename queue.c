#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

static inline element_t *new_element(const char *s)
{
    if (!s)
        return NULL;
    element_t *new = malloc(sizeof(element_t));
    char *temp = strdup(s);
    if (!new || !temp) {
        free(new);
        free(temp);
        return NULL;
    }
    new->value = temp;
    return new;
}


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
    if (!l)
        return;
    element_t *temp, *it;
    list_for_each_entry_safe (it, temp, l, list)
        q_release_element(it);
    free(l);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new = new_element(s);
    if (!new)
        return false;
    list_add(&new->list, head);
    return true;
}


/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new = new_element(s);
    if (!new)
        return false;
    list_add_tail(&new->list, head);
    return true;
}



/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *remove_loc = list_first_entry(head, element_t, list);
    if (!remove_loc)
        return NULL;
    list_del(&remove_loc->list);
    if (sp && bufsize) {
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
    if (sp && bufsize) {
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
    struct list_head *r = head->next;
    struct list_head *l = head->prev;
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
    if (!head)
        return;
    struct list_head *node;
    for (node = head->next; node != head && node->next != head;
         node = node->next) {
        struct list_head *next = node->next;
        list_del_init(node);
        list_add(node, next);
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    struct list_head *cur = head, *next = head->next;
    do {
        cur->next = cur->prev;
        cur->prev = next;
        cur = next;
        next = cur->next;
    } while (cur != head);
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
        node = strcmp(E1->value, E2->value) <= 0 ? &L1 : &L2;
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
    head->prev->next = NULL;
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
    if (!head || list_empty(head))
        return 0;
    element_t *it, *safe;
    char *max = NULL;
    int count = 0;
    for (it = list_entry(head->prev, element_t, list),
        safe = list_entry(head->prev->prev, element_t, list);
         &it->list != head;
         it = safe, safe = list_entry(safe->list.prev, element_t, list)) {
        if (!max || strcmp(it->value, max) > 0) {
            count++;
            max = it->value;
        } else {
            list_del(&it->list);
            q_release_element(it);
        }
    }
    return count;
}

void merge_two_list(struct list_head *L1, struct list_head *L2)
{
    if (!L1 || !L2)
        return;
    struct list_head head;
    INIT_LIST_HEAD(&head);
    while (!list_empty(L1) && !list_empty(L2)) {
        element_t *E1 = list_first_entry(L1, element_t, list);
        element_t *E2 = list_first_entry(L2, element_t, list);
        element_t *node = strcmp(E1->value, E2->value) <= 0 ? E1 : E2;
        list_move_tail(&node->list, &head);
    }
    list_splice_tail_init(L1, &head);
    list_splice_tail(L2, &head);
    list_splice(&head, L1);
}

/* Merge all the queues into one sorted queue, which is in ascending order */
int q_merge(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return q_size(list_first_entry(head, queue_contex_t, chain)->q);
    queue_contex_t *it, *safe;
    // remove empty chain to avoid "while (first->q && second->q)" miss one
    // of no empty queue. e.g. lists = [[1,4,5],[1,3,4],[],[2,6]] , because when
    // first==[] second==[2,6], [2,6] be neglect
    list_for_each_entry_safe (it, safe, head, chain) {
        if (!it->q)
            list_del_init(&it->chain);
    }
    int count = (q_size(head) + 1) / 2;
    for (int i = 0; i < count; i++) {
        queue_contex_t *first = list_first_entry(head, queue_contex_t, chain);
        queue_contex_t *second =
            list_entry(first->chain.next, queue_contex_t, chain);
        while (first->q && second->q) {
            merge_two_list(first->q, second->q);
            second->q = NULL;
            list_move_tail(&second->chain, head);
            first = list_entry(first->chain.next, queue_contex_t, chain);
            second = list_entry(first->chain.next, queue_contex_t, chain);
        }
    }
    return q_size(list_first_entry(head, queue_contex_t, chain)->q);
}


/* Copyright (c) 2013-2019 GomSpace A/S. All rights reserved. */

#include "fp_list.h"
#include <gs/util/stdio.h>
#include <gs/util/string.h>
#include <stdlib.h>

bool fp_list_empty(fp_list_t * list)
{
    return (!list || !list->head || !list->tail);
}

/** List initialisation */
gs_error_t fp_list_init(fp_list_t * list)
{
    if (list == NULL)
        return GS_ERROR_HANDLE;

    /* Reset list pointers */
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;

    return GS_OK;
}

gs_error_t fp_list_insert(fp_list_t * list, int pos, const char * key, void * entry)
{
    gs_error_t ret = GS_ERROR_HANDLE;

    if (list == NULL || entry == NULL)
        return GS_ERROR_HANDLE;

    fp_list_entry_t * insert = (fp_list_entry_t*)calloc(1, sizeof(*insert));
    if (insert == NULL)
        return GS_ERROR_ALLOC;

    insert->entry = entry;
    strncpy(insert->key, key, FP_KEY_SIZE);
    insert->key[FP_KEY_SIZE - 1] = '\0';

    if (fp_list_empty(list)) {
        /* Insert in empty list */
        list->head = insert;
        list->tail = insert;
        insert->next = NULL;
        insert->prev = NULL;
        list->count++;
        ret = GS_OK;
    } else if (pos >= list->count) {
        /* Append to list tail */
        insert->next = NULL;
        insert->prev = list->tail;
        list->tail->next = insert;
        list->tail = insert;
        list->count++;
        ret = GS_OK;
    } else {
        /* Find position */
        int i = 0;
        fp_list_entry_t * tmp = list->head;
        while (tmp) {
            if (i++ == pos) {
                /* Insert item */
                insert->next = tmp;
                insert->prev = tmp->prev;

                /* Update list head */
                if (tmp->prev == NULL)
                    list->head = insert;
                else
                    tmp->prev->next = insert;

                /* Update list tail */
                if (tmp->next == NULL)
                    list->tail = insert;
                else
                    tmp->prev = insert;

                list->count++;
                ret = GS_OK;
                break;
            }
            tmp = tmp->next;
        }
    }

    return ret;
}

gs_error_t fp_list_insert_unique(fp_list_t * list, int pos, const char * key, void * entry)
{
    /* Search for entry in list */
    void * test = fp_list_search(list, key);
    if (test != NULL) {
        return GS_ERROR_EXIST;
    }

    return fp_list_insert(list, pos, key, entry);

}

gs_error_t fp_list_append(fp_list_t * list, const char * key, void * entry)
{
    /* Insert in last position of list */
    return fp_list_insert(list, list->count, key, entry);
}

gs_error_t fp_list_append_unique(fp_list_t * list, const char * key, void * entry)
{
    /* Insert in last position of list */
    return fp_list_insert_unique(list, list->count, key, entry);
}

void * fp_list_search(fp_list_t * list, const char * key)
{
    if (list == NULL)
        return NULL;

    /* List empty */
    if (fp_list_empty(list))
        return NULL;

    fp_list_entry_t * tmp = list->head;
    while (tmp) {
        /* Entry found */
        if (strncmp(tmp->key, key, FP_KEY_SIZE) == 0)
            return tmp->entry;

        tmp = tmp->next;
    }

    return NULL;
}

void * fp_list_get(fp_list_t * list, int pos)
{
    if (list == NULL || fp_list_empty(list) || pos >= list->count)
        return NULL;

    int i = 0;
    fp_list_entry_t * tmp = list->head;
    while (tmp) {
        if (i++ == pos)
            return tmp->entry;
        tmp = tmp->next;
    }

    return NULL;
}

fp_list_entry_t * fp_list_remove(fp_list_t * list, const char * key)
{
    if (list == NULL)
        return NULL;

    /* List empty */
    if (fp_list_empty(list))
        return NULL;

    fp_list_entry_t * tmp = list->head;
    while (tmp) {
        if (strncmp(tmp->key, key, FP_KEY_SIZE) == 0) {
            /* Last element */
            if (tmp->next)
                tmp->next->prev = tmp->prev;
            else
                list->tail = tmp->prev;

            /* First element */
            if (tmp->prev)
                tmp->prev->next = tmp->next;
            else
                list->head = tmp->next;

            list->count--;
            return tmp;
        }
        tmp = tmp->next;
    }

    return NULL;
}

int fp_list_count(fp_list_t * list)
{
    return (list == NULL) ? -1 : list->count;
}

gs_error_t fp_list_iterate(fp_list_t * list, fp_iter_func_t apply, void * ret, size_t retsize)
{
    if (list == NULL)
        return GS_ERROR_HANDLE;

    fp_list_entry_t * tmp = list->head;
    int pos = 0;
    while (tmp) {
        if (apply(pos, tmp->key, tmp->entry, ret, retsize)) {
            return GS_ERROR_UNKNOWN;
        }
        tmp = tmp->next;
        pos++;
    }

    return GS_OK;
}

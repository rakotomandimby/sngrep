/**************************************************************************
 **
 ** sngrep - SIP Messages flow viewer
 **
 ** Copyright (C) 2015 Ivan Alonso (Kaian)
 ** Copyright (C) 2015 Irontec SL. All rights reserved.
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **
 ****************************************************************************/
/**
 * @file vector.c
 * @author Ivan Alonso [aka Kaian] <kaian@irontec.com>
 *
 * @brief Source code of functions defined in vector.h
 *
 */
#include "vector.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

vector_t *
vector_create(int limit, int step)
{
    vector_t *v;
    // Allocate memory for this vector data
    if (!(v = malloc(sizeof(vector_t))))
        return NULL;

    memset(v, 0, sizeof(vector_t));
    v->limit = limit;
    v->step = step;

    // Initial memory allocation
    if (!(v->list = malloc(sizeof(void *) * limit))) {
        free(v);
        return NULL;
    }

    // Return vector pointer
    return v;
}

void
vector_clear(vector_t *vector)
{
    vector->count = 0;
}

int
vector_append(vector_t *vector, void *item)
{
    // Sanity check
    if (!item)
        return vector->count;

    // Check if we need to increase vector size
    if (vector->count == vector->limit) {
        // Increase vector size
        vector->limit += vector->step;
        // Add more memory to the list
        vector->list = realloc(vector->list, sizeof(void *) * vector->limit);
    }
    // Add item to the end of the list
    vector->list[vector->count++] = item;
    return vector->count;
}

void
vector_remove(vector_t *vector, void *item)
{
    // Get item position
    int idx = vector_index(vector, item);
    // Decrease item counter
    vector->count--;
    // Move the rest of the elements one position up
    memcpy(vector->list + idx, vector->list + idx + 1, sizeof(void *) * (vector->count - idx));
}

void *
vector_item(vector_t *vector, int index)
{
    if (index >= vector->count || index < 0)
        return NULL;
    return vector->list[index];
}

void *
vector_first(vector_t *vector)
{
    return vector_item(vector, 0);
}

int
vector_index(vector_t *vector, void *item)
{
    // FIXME Bad perfomance
    int i;
    for (i = 0; i < vector->count; i++) {
        if (vector->list[i] == item)
            return i;
    }
    return -1;
}

int
vector_count(vector_t *vector)
{
    return vector->count;
}

vector_iter_t
vector_iterator(vector_t *vector)
{
    vector_iter_t it;
    memset(&it, 0, sizeof(vector_iter_t));
    it.current = -1;
    it.vector = vector;
    return it;
}

vector_t *
vector_iterator_vector(vector_iter_t *it)
{
    return it->vector;
}

int
vector_iterator_count(vector_iter_t *it)
{
    int count = 0;
    int pos = it->current;

    vector_iterator_reset(it);

    if (!it->filter) {
        count = vector_count(it->vector);
    } else {
        while (vector_iterator_next(it)) {
            count++;
        }
    }

    vector_iterator_set_current(it, pos);

    return count;
}

void *
vector_iterator_next(vector_iter_t *it)
{
    void *item;

    if (it->current >= vector_count(it->vector))
        return NULL;

    while ((item = vector_item(it->vector, ++it->current))) {
        if (it->filter) {
            if (it->filter(item)) {
                return item;
            }
        } else {
            return item;
        }
    }
    return NULL;
}

void *
vector_iterator_prev(vector_iter_t *it)
{
    void *item;

    if (it->current == -1)
        return NULL;

    while ((item = vector_item(it->vector, --it->current))) {
        if (it->filter) {
            if (it->filter(item)) {
                return item;
            }
        } else {
            return item;
        }
    }
    return NULL;
}

void
vector_iterator_set_filter(vector_iter_t *it, int
(*filter)(void *item))
{
    it->filter = filter;
}

void
vector_iterator_set_current(vector_iter_t *it, int current)
{
    it->current = current;
}

int
vector_iterator_current(vector_iter_t *it)
{
    return it->current;
}

void
vector_iterator_reset(vector_iter_t *it)
{
    vector_iterator_set_current(it, -1);
}


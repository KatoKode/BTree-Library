/*------------------------------------------------------------------------------
    BTree Implementation in x86_64 Assembly Language with C Interface
    Copyright (C) 2025  J. McIntosh

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
------------------------------------------------------------------------------*/
#ifndef B_TREE_H
#define B_TREE_H

#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../util/util.h"

#define BT_FALSE   0
#define BT_TRUE    1

#define NODE_COUNT  512

extern uint8_t simd_level;

typedef int (*b_compare_cb) (void const *, void const *);
typedef void (*b_delete_cb) (void const *);
typedef void const * (*b_get_key_cb) (void const *);
typedef void (*b_walk_cb) (void const *);
typedef void * (*b_get_obj_cb) (void);

typedef struct b_tree b_tree_t;

typedef struct b_node b_node_t;

struct b_node {
  size_t        nobj;   // number of objects
  b_tree_t *    tree;   // tree pointer
  b_node_t **   child;  // array of child pointers
  void *        object; // array of objects
  uint8_t       leaf;   // 0 (false) | 1 (true)
};

#define b_node_alloc() (calloc(1, sizeof(b_node_t)))
#define b_node_free(P) (free(P), P = NULL)

struct b_tree {
  size_t        mindeg;   // minimum-degree of the tree
  size_t        o_size;   // ex: o_size = 16 aligned to 16
                          // ex: o_size = 20 aligned to 24
  b_compare_cb  o_cmp_cb; // user supplied function
  b_compare_cb  k_cmp_cb; // user supplied function
  b_delete_cb   o_del_cb; // user supplied function
  b_get_key_cb  k_get_cb; // user supplied function
  b_node_t *    root;
};

#define b_tree_alloc() (calloc(1, sizeof(b_tree_t)))
#define b_tree_free(P) (free(P), P = NULL)

#define B_MAX_LEVEL     64

typedef struct b_iter b_iter_t;

struct b_iter {
  b_node_t *    node [B_MAX_LEVEL];
  size_t        nobj [B_MAX_LEVEL];
  size_t        depth;
  b_tree_t *    tree;
  void *        curr;
};

#define b_iter_alloc() (calloc(1, sizeof(b_iter_t)))
#define b_iter_free(P) (free(P), P = NULL)

b_node_t ** b_child_at (b_node_t *, size_t const);
void * b_object_at (b_node_t *, size_t const);
size_t b_find_key (b_node_t *, void const *, int *);
int b_insert (b_tree_t *, void const *);
void b_node_init (b_node_t *, b_tree_t *, uint8_t const );
void b_node_term (b_node_t *);
int b_remove (b_tree_t *, void const *);
void * b_search (b_node_t *, void const *);
void b_tree_init (b_tree_t *, size_t const, size_t const, b_compare_cb,
    b_compare_cb, b_delete_cb, b_get_key_cb);
void b_tree_term (b_tree_t *);
void b_walk (b_tree_t *, b_walk_cb);

b_iter_t * b_iter_begin (b_tree_t *);
void * b_iter_deref(const b_iter_t *); 
b_iter_t * b_iter_end (b_tree_t *);
int b_iter_eq(const b_iter_t *, const b_iter_t *);
int b_iter_init (b_iter_t *, b_tree_t *);
void b_iter_next (b_iter_t *);
void b_iter_term(b_iter_t *);
int b_iter_valid (const b_iter_t *);

void b_bulk_load (b_tree_t *, b_get_obj_cb);

b_iter_t * b_lower_bound (b_tree_t *, void const *);
b_iter_t * b_upper_bound (b_tree_t *, void const *);

void detect_simd_features(void);
#endif

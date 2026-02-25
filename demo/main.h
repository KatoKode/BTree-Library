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
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include "../btree/btree.h"
#include "../util/util.h"
#include "pcg/pcg_basic.h"

__thread pcg32_random_t thread_rng;
__thread int thread_seeded = 0;

size_t my_rand = 0;

//#define DEMO_DEBUG  1
//#define WALK_TREE   1
//#define TREE_ITER   1

// defines you can modify
#define DATA_COUNT      (8192 * 1024)
#define DELETE_COUNT    (DATA_COUNT * 0.30)
#define MINIMUM_DEGREE  48

// index for tree walking
size_t ndx;
size_t o_del_count;
size_t bulk_count;

// data object
typedef struct data data_t;

struct data {
  long      lng;
};

data_t bulk_obj = { .lng = 0 };

long la [DATA_COUNT];

// callback definitions
int o_cmp_cb (void const *, void const *);
int k_cmp_cb (void const *, void const *);
void const * k_get_cb (void const *);
void o_del_cb (void const *);
void walk_cb (void const *);
// output data_t object
void print_data (char const *, data_t const *);
// begin termination of tree
void term_tree (b_tree_t *);
// begin walking the tree
void walk_tree (b_tree_t *);
// get object for bulk load
void * bulk_get_object(void);
// run bluk load
void bulk_load (b_tree_t *, b_get_obj_cb);
// run mixed insert/delete ops
void mixed_insert_delete (b_tree_t *);
// run bulk insert/delete ops
void bulk_insert_delete (b_tree_t *);
// run range scan
void range_scan (b_tree_t *);


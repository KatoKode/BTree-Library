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
#include "main.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage:\n\t./demo ./sorted.txt 1    OR\n" \
            "\t./demo ./unsorted.txt 2  OR\n" \
            "\t./demo ./unsorted.txt 3  OR\n" \
            "\t./demo ./sorted.txt 4\n");
        return -1;
    }

    FILE *infile = fopen(argv[1], "r");
    if (!infile) {
        perror("fopen");
        return -1;
    }

    size_t line_size = 32;
    char *line = calloc(1, line_size);
    for (size_t i = 0; i < DATA_TOTAL; ++i) {
        size_t nread = getline(&line, &line_size, infile);
        if (nread == (size_t)-1) break;
        line[nread - 1] = '\0';
        la[i] = strtol(line, NULL, 10);
    }
    free(line);
    fclose(infile);


    // Tree configuration
    size_t const mindeg = MINIMUM_DEGREE;
    size_t const o_size = sizeof(data_t);

    b_tree_t *tree = b_tree_alloc();
    b_tree_init(tree, mindeg, o_size, o_cmp_cb, k_cmp_cb, o_del_cb, k_get_cb);

    int n = atoi(argv[2]);

    switch (n)
    {
      case 1: bulk_load(tree, bulk_get_object);
              break;
      case 2: bulk_insert_delete(tree);
              break;
      case 3: mixed_insert_delete(tree);
              break;
      case 4: bulk_load(tree, bulk_get_object); range_scan(tree);
              break;
      default:;
    }

#ifdef WALK_TREE
    walk_tree(tree);
#endif

#ifdef TREE_ITER
    b_iter_t *end = b_iter_end(tree);
    
    for (b_iter_t *iter = b_iter_begin(tree);
        ! b_iter_eq(iter, end);
        b_iter_next(iter))
      walk_cb(b_iter_deref(iter));
#endif
    term_tree(tree);
    return 0;
}
//------------------------------------------------------------------------------
//
// O_CMP_CB
//
//------------------------------------------------------------------------------
int o_cmp_cb (void const *vp1, void const *vp2) {
  data_t const *d1 = vp1;
  data_t const *d2 = vp2;
  // do comparsions
  if (d1->lng > d2->lng) return 1;
  else if (d1->lng < d2->lng) return -1;
  return 0;
}
//------------------------------------------------------------------------------
//
// K_CMP_CB
//
//------------------------------------------------------------------------------
int k_cmp_cb (void const * vp1, void const * vp2) {
  long const lng = *(long const *)vp1;
  data_t const *d2 = vp2;
  // do comparsions
  if (lng > d2->lng) return 1;
  else if (lng < d2->lng) return -1;
  return 0;
}
//------------------------------------------------------------------------------
//
// K_GET_CB
//
//------------------------------------------------------------------------------
void const * k_get_cb (void const *vp) {
  data_t const *dp = vp;
  // return object key
  return &dp->lng;
}
//------------------------------------------------------------------------------
//
// O_DEL_CB
//
//------------------------------------------------------------------------------
void o_del_cb (void const *vp) {
}
//------------------------------------------------------------------------------
//
// TERM_TREE
//
//------------------------------------------------------------------------------
void term_tree (b_tree_t *tree) {
  b_tree_term(tree);
  b_tree_free(tree);
}
//------------------------------------------------------------------------------
//
// WALK_CB
//
//------------------------------------------------------------------------------
void walk_cb (void const *vp) {
  data_t const *d = vp;

  printf("%6lu:  lng: %ld\n", ndx++, d->lng);

  fflush(stdout);
}
//------------------------------------------------------------------------------
//
// WALK_TREE
//
//------------------------------------------------------------------------------
void walk_tree (b_tree_t *tree) {
  puts("\n---| walk tree |---\n");

  // initialize index used by tree walking callback
  ndx = 0L;

  b_walk(tree, walk_cb);
}
//------------------------------------------------------------------------------
//
// BULK_GET_OBJECT
//
//------------------------------------------------------------------------------
void * bulk_get_object(void) {
  if (bulk_count >= DATA_TOTAL) return NULL;
  bulk_obj.lng = la[bulk_count++];
  return &bulk_obj;
}
//------------------------------------------------------------------------------
//
// BULK_LOAD
//
//------------------------------------------------------------------------------
void bulk_load (b_tree_t *tree, b_get_obj_cb get_object)
{
  printf("┌─────────────────────────┐\n");
  printf("│        BULK LOAD        │\n");
  printf("└─────────────────────────┘\n");

  b_bulk_load(tree, get_object);
}
//------------------------------------------------------------------------------
//
// MIXED_INSERT_DELETE
//
//------------------------------------------------------------------------------
void mixed_insert_delete (b_tree_t *tree)
{
  printf("┌─────────────────────────┐\n");
  printf("│ MIXED INSERT/DELETE OPS │\n");
  printf("└─────────────────────────┘\n");

  size_t total_ops = 0;
  size_t insert_count = 0;
  size_t delete_count = 0;
  size_t DELETE_TOTAL = DATA_TOTAL * 0.3;

  printf("Starting mixed inserts/deletes workload...\n\n");

  // === Timing starts here ===
  struct timespec start, stop;
  if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
    perror("clock_gettime start failed");
    return;
  }

  while (total_ops < (DATA_TOTAL + DELETE_TOTAL)) {
    if ((total_ops % 10) < 7) {
      data_t d;
      d.lng = la[insert_count++];

      if (b_insert(tree, &d) == 0) {
        total_ops++;
      }
    } else {
      if (tree->root && tree->root->nobj > 0 && insert_count > delete_count) {
        long candidate = la[delete_count++];
        b_remove(tree, (void const *)&candidate);
        total_ops++;
      }
    }
  }

  // === Timing ends here ===
  if (clock_gettime(CLOCK_MONOTONIC, &stop) == -1) {
    perror("clock_gettime stop failed");
  }

  double elapsed_sec = (stop.tv_sec - start.tv_sec)
    + (stop.tv_nsec - start.tv_nsec) / 1e9;

  printf("\n=== Timed Results ===\n");
  printf("Total operations: %zu inserts = %zu deletes = %zu\n", total_ops,
      insert_count, delete_count);
  printf("Elapsed time: %.3f seconds\n", elapsed_sec);
  printf("Throughput: %.0f ops/sec\n\n",
      (insert_count + delete_count) / elapsed_sec);
}
//------------------------------------------------------------------------------
//
// BULK_INSERT_DELETE
//
//------------------------------------------------------------------------------
void bulk_insert_delete (b_tree_t *tree)
{
  printf("┌────────────────────────┐\n");
  printf("│ BULK INSERT/DELETE OPS │\n");
  printf("└────────────────────────┘\n");

  size_t total_ops = 0;
  size_t insert_count = 0;
  size_t delete_count = 0;
  size_t array_idx = 0;
  size_t DELETE_TOTAL = DATA_TOTAL * 0.75;

  printf("Starting %lu inserts workload...\n\n", (size_t)DATA_TOTAL);

  // === Timing starts here ===
  struct timespec start, stop;
  if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
    perror("clock_gettime start failed");
    return;
  }

  while (insert_count < (size_t)DATA_TOTAL) {
    data_t d;
    d.lng = la[array_idx++];

    if (b_insert(tree, &d) == 0) {
      insert_count++;
      total_ops++;
    }
  }

  // === Timing ends here ===
  if (clock_gettime(CLOCK_MONOTONIC, &stop) == -1) {
    perror("clock_gettime stop failed");
  }

  double elapsed_sec = (stop.tv_sec - start.tv_sec)
    + (stop.tv_nsec - start.tv_nsec) / 1e9;

  printf("\n=== Timed Results ===\n");
  printf("Total operations: %zu inserts = %zu\n", insert_count, insert_count);
  printf("Elapsed time: %.3f seconds\n", elapsed_sec);
  printf("Throughput: %.0f ops/sec\n\n", insert_count / elapsed_sec);

  printf("Starting %lu deletes workload...\n\n", (size_t)DELETE_TOTAL);

  // === Timing starts here ===
  if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
    perror("clock_gettime start failed");
    return;
  }

  while (delete_count < DELETE_TOTAL) {
    long candidate = la[delete_count++];
    b_remove(tree, (void const *)&candidate);
    total_ops++;
  }

  // === Timing ends here ===
  if (clock_gettime(CLOCK_MONOTONIC, &stop) == -1) {
    perror("clock_gettime stop failed");
  }

  elapsed_sec = (stop.tv_sec - start.tv_sec)
    + (stop.tv_nsec - start.tv_nsec) / 1e9;

  printf("\n=== Timed Results ===\n");
  printf("Total operations: %zu deletes = %zu\n", delete_count, delete_count);
  printf("Elapsed time: %.3f seconds\n", elapsed_sec);
  printf("Throughput: %.0f ops/sec\n", delete_count / elapsed_sec);
}
//------------------------------------------------------------------------------
//
// RANGE_SCAN
//
//------------------------------------------------------------------------------
void range_scan (b_tree_t *tree)
{
  printf("┌────────────────────────┐\n");
  printf("│       RANGE SCAN       │\n");
  printf("└────────────────────────┘\n");

  long int key1 = 1670491;
  long int key2 = 1670510;
  b_iter_t *lower = b_lower_bound(tree, &key1);
  b_iter_t *upper = b_upper_bound(tree, &key2);
  for (; !b_iter_eq(lower, upper); b_iter_next(lower))
    printf("key: %lu\n", *((long int *)(b_iter_deref(lower))));
  b_iter_term(upper);
  b_iter_free(upper);
  b_iter_term(lower);
  b_iter_free(lower);
}


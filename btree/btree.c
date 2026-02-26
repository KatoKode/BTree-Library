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
#include "btree.h"

size_t b_find_key (b_node_t *, void const *, int *);

// ─────────────────────────────────────────────────────────────────────────────
//
// initBTreeLibrary
//
// ─────────────────────────────────────────────────────────────────────────────
void __attribute__ ((constructor)) initBTreeLibrary(void)
{
  detect_simd_features();
}
// ─────────────────────────────────────────────────────────────────────────────
//
// termBTreeLibrary
//
// ─────────────────────────────────────────────────────────────────────────────
void __attribute__ ((destructor)) termBTreeLibrary(void)
{
}
// ─────────────────────────────────────────────────────────────────────────────
//
// B_BULK_LOAD
//
// ─────────────────────────────────────────────────────────────────────────────
void b_bulk_load (b_tree_t *tree, b_get_obj_cb get_object)
{
  size_t child_total = NODE_COUNT;
  size_t child_count = 0;

  b_node_t **child_array = malloc(child_total * sizeof(b_node_t*));

  uint8_t leaf = 1;
  b_node_t *node = b_node_alloc();
  b_node_init(node, tree, leaf);

  void *object = NULL;
  for (object = get_object();
      object != NULL;
      object = get_object())
  {
    if (node->nobj >= 2 * tree->mindeg - 1)
    {
      child_array[child_count++] = node;

      if (child_count >= child_total)
      {
        child_total += NODE_COUNT;
        child_array = realloc(child_array, child_total * sizeof(b_node_t*));
      }

      node = b_node_alloc();
      b_node_init(node, tree, leaf);
    }

    void *dst = (void *)((size_t)node->object + node->nobj * tree->o_size);

    memmove(dst, object, tree->o_size);

    node->nobj += 1;
  }

  child_array[child_count++] = node;

  size_t object_count = 0;
  for (size_t i = 0; i < child_count; ++i)
  {
    object_count += child_array[i]->nobj;
  }

  printf("object_count: %lu\n", object_count);


  leaf = 0;

  do {
    size_t parent_total = NODE_COUNT;
    size_t parent_count = 0;
    b_node_t **parent_array = malloc(parent_total * sizeof(b_node_t*));

    size_t i = 0;           // index into current child_array

    while (i < child_count)
    {
      b_node_t *parent = b_node_alloc();
      b_node_init(parent, tree, leaf);

      // Always start with at least one child (the leftmost)
      parent->child[0] = child_array[i];
      i++;

      // Now add (key, child) pairs until the node is full or no more children
      while (i < child_count && parent->nobj < 2 * tree->mindeg - 1)
      {
        // Take the **last** key from the previous child as separator
        b_node_t *prev_child = child_array[i - 1];
        void *src = (char *)prev_child->object + (prev_child->nobj - 1) * tree->o_size;
        void *dst = (char *)parent->object + parent->nobj * tree->o_size;
        memcpy(dst, src, tree->o_size);   // memcpy is fine here (no overlap)

        // Moved prev_child->object[prev_child->nobj - 1] up one level, so
        // decrement object count
        prev_child->nobj--;

        // Add the next child after this key
        parent->child[parent->nobj + 1] = child_array[i];
        parent->nobj++;
        i++;
      }

      // The Key to makeing classic B-Tree bottom-up bluk load possible.
      if (i < child_count) --i;

      // Store completed parent
      parent_array[parent_count++] = parent;

      if (parent_count >= parent_total)
      {
        parent_total += NODE_COUNT;
        parent_array = realloc(parent_array, parent_total * sizeof(b_node_t*));
      }
    }

    // Debugging print (optional - remove later)
    printf("Level done → child_count was %zu, produced %zu parents\n",
        child_count, parent_count);

    // Prepare for next level
    free(child_array);               // free previous level's array (not the nodes!)
    child_array = parent_array;
    child_count = parent_count;

  } while (child_count > 1);

  // After loop: the last level should have exactly 1 node → that's the root
  if (child_count == 1) {
    tree->root = child_array[0];
    free(child_array);
  } else if (child_count == 0) {
    // empty tree case — handle if needed
    tree->root = NULL;
  } else {
    fprintf(stderr, "Bug: final level has %zu nodes (expected 1)\n", child_count);
  }
}
// ─────────────────────────────────────────────────────────────────────────────
//
// B_LOWER_BOUND_RECURS
//
// ─────────────────────────────────────────────────────────────────────────────
b_iter_t * b_lower_bound_recurs (b_node_t *node, b_iter_t *iter, void const *key)
{
  int cond;
  size_t i = b_find_key(node, key, &cond);

  iter->node[iter->depth] = node;
  iter->nobj[iter->depth] = i;

  if (node->leaf == BT_TRUE || (i < node->nobj && cond == 0))
  {
    b_iter_next(iter);
    return iter;
  }

  iter->depth++;

  return b_lower_bound_recurs(node->child[i], iter, key);
}
// ─────────────────────────────────────────────────────────────────────────────
//
// B_LOWER_BOUND
//
// ─────────────────────────────────────────────────────────────────────────────
b_iter_t * b_lower_bound (b_tree_t *tree, void const *key)
{
  if (tree->root == NULL) return b_iter_end(tree);

  b_iter_t *iter = b_iter_alloc();
  b_iter_init(iter, tree);

  return b_lower_bound_recurs(tree->root, iter, key);
}
// ─────────────────────────────────────────────────────────────────────────────
//
// B_UPPER_BOUND_RECURS
//
// ─────────────────────────────────────────────────────────────────────────────
b_iter_t * b_upper_bound_recurs (b_node_t *node, b_iter_t *iter, void const *key)
{
  int cond;
  size_t i = b_find_key(node, key, &cond);

  iter->node[iter->depth] = node;
  iter->nobj[iter->depth] = i;

  if (node->leaf == BT_TRUE || (i < node->nobj && cond == 0))
  {
    b_iter_next(iter);
    if (cond == 0) b_iter_next(iter);
    return iter;
  }

  iter->depth++;

  return b_upper_bound_recurs(node->child[i], iter, key);
}
// ─────────────────────────────────────────────────────────────────────────────
//
// B_UPPER_BOUND
//
// ─────────────────────────────────────────────────────────────────────────────
b_iter_t * b_upper_bound (b_tree_t *tree, void const *key)
{
  if (tree->root == NULL) return b_iter_end(tree);

  b_iter_t *iter = b_iter_alloc();
  b_iter_init(iter, tree);

  return b_upper_bound_recurs(tree->root, iter, key);
}


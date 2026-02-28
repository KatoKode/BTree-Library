
[![License: GPL-2.0](https://img.shields.io/badge/License-GPL%202.0-blue.svg)](https://opensource.org/licenses/GPL-2.0)
[![Stars](https://img.shields.io/github/stars/KatoKode/BTree-Library?style=social)](https://github.com/KatoKode/BTree-Library/stargazers)

# libbtree — High-Performance B-Tree Shared Library

**Author:** JD McIntosh
**Current status:** Pre-built shared library + minimal demo (2026)

A fast, generic, in-memory **B-Tree** implementation with critical paths in x86-64 assembly, exposed as a clean C shared library (`libbtree.so`).

- Valgrind **clean** (zero leaks, zero invalid reads/writes in extensive testing)
- Broad Linux compatibility (glibc ≥ 2.14 → runs on Ubuntu 18.04+, Debian 10+, RHEL 7+/AlmaLinux 8+/Rocky 8+, openSUSE Leap 15+, SUSE SLES 12+, Fedora, etc.)
- SIMD-aware memory moves (scalar / SSE2 / AVX2 detection at startup)
- Designed for high-throughput insertion, deletion, search with proper balancing

Workload                  | Ops/sec (avg) | Notes
--------------------------|---------------|------
Bulk load 8M keys         | ~1B equiv?   | 0.008s total
Random inserts (1.25M)    | 2.19M        | Single-threaded
Random deletes (0.94M)    | 4.11M        | 
Mixed insert/delete (~1.6M) | 2.78M      | 70/30 split

hardware: i7-11800H @ ~4.0–4.6 GHz turbo, using 8-byte keys.

## Features

- **Generic objects** — arbitrary payload size (`o_size`), automatically aligned to 8 bytes
- **Configurable minimum degree** (`mindeg` ≥ 2)
- **Separate comparison callbacks**:
  - `o_cmp_cb` — full object vs full object
  - `k_cmp_cb` — key vs full object (for search/delete)
- **Key extraction** via `k_get_cb` (enables key-only lookups)
- **Custom cleanup** via `o_del_cb` (called on delete/terminate)
- **No duplicates** — insert returns -1 if key already exists
- **Full balancing** — split on insert overflow, borrow/merge on delete underflow
- **Iterator API** — in-order traversal (`b_iter_begin`, `b_iter_next`, `b_iter_deref`, etc.)
- **Lower/upper bound** support (`b_lower_bound`, `b_upper_bound`)
- **Bulk load** helper (`b_bulk_load`) for sorted/pre-sorted data
- **Optimized small-node search** — linear hunt ≤9 items, binary search above
- **Custom aligned memmove** in assembly with runtime SIMD dispatch

## Included Files

- `libbtree.so`     — pre-built shared library (Valgrind clean, portable glibc baseline)
- `btree.h`         — public C header with all API declarations
- `main.h`          — demo configuration & data structures
- `main.c`          — example program (insert, delete, search, iterate, bulk load)
- `makefile`        — builds the demo linking against `libbtree.so`
- `sorted.txt`      — sorted text input file
- `unsorted.txt`    — unsorted text input file

## Quick Start — Build & Run Demo

Run the following command in the `BTree-Library-main` folder:

```bash
sh ./btree_make.sh
```

In folder `demo` enter the following command:

```bash
./go_demo.sh
```

## Adjustable Minimum Degree >= 2
```C
#define MINIMUM_DEGREE  2              // try 32 or 48 or 64 for shallower tree
```

## C Defs in btree.h

```C
int b_insert (b_tree_t *, void const *);
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
```

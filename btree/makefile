#-------------------------------------------------------------------------------
#   BTree Implementation in x86_64 Assembly Language with C Interface
#   Copyright (C) 2025  J. McIntosh
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program; if not, write to the Free Software Foundation, Inc.,
#   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#-------------------------------------------------------------------------------
#
# Recommended hardening flags for GCC 7.5 on openSUSE Leap 15.4
#
CFLAGS   := -O3 -march=native -m64 -mno-avx512f -flto -Wall \
						-Werror=format-security -D_FORTIFY_SOURCE=2 \
						-fstack-protector-strong -fasynchronous-unwind-tables \
						-fexceptions -pipe

LDFLAGS  := -O3 -Wl,-z,relro -Wl,-z,now -Wl,-z,defs
# Optional: add -z noexecstack if you have hand-written asm without proper .note.GNU-stack
# LDFLAGS  += -O3 -Wl,-z,noexecstack

ASMFLAGS := -Ox

.PHONY: all clean

all: libbtree.so

libbtree.so: btree_asm.o btree.o
	gcc $(LDFLAGS) -shared -o $@ $^

btree.o: btree.c
	gcc $(CFLAGS) -fPIC -c $< -o $@

btree_asm.o: btree.asm
	nasm $(ASMFLAGS) -f elf64 $< -o $@

clean:
	rm -f libbtree.so *.o

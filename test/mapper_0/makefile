# NESL
# Copyright (C) 2022 David Jolly
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
# associated documentation files (the "Software"), to deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge, publish, distribute,
# sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all copies or
# substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
# PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
# AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

DIR_ROOT=./
DIR_SRC=../../src/system/mapper/

FILE=mapper_0
FILE_BIN=$(DIR_ROOT)nesl-$(FILE)
FILES_OBJ=$(patsubst $(DIR_ROOT)%.c,$(DIR_ROOT)%.o,$(FILES_SRC))
FILES_SRC=$(shell find $(DIR_ROOT) -name '*.c')

FLAGS_LIB=-lSDL2 -lSDL2main

.PHONY: all
all: build

.PHONY: build
build: $(FILE_BIN)

.PHONY: run
run:
	@if ! ./$(FILE_BIN); then exit 1; fi

.PHONY: clean
clean:
	rm -rf $(FILE_BIN)
	rm -rf $(FILES_OBJ)

$(DIR_SRC)$(FILE).o: $(DIR_SRC)$(FILE).c
	$(CC) $(FLAGS) -c -o $@ $<

$(DIR_ROOT)%.o: $(DIR_ROOT)%.c
	$(CC) $(FLAGS) -c -o $@ $<

$(FILE_BIN): $(DIR_SRC)$(FILE).o $(FILES_OBJ)
	$(CC) $(FLAGS) $(DIR_SRC)$(FILE).o $(FILES_OBJ) $(FLAGS_LIB) -o $@

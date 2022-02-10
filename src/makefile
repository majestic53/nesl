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

DIR_INCLUDE=../include/
DIR_BUILD=../build/
DIR_SRC=./

FILE_BIN=$(DIR_BUILD)nesl
FILE_LIB=$(DIR_BUILD)libnesl.a
FILE_SRC=$(shell find $(DIR_SRC) -name '*.c')
FILE_OBJ=$(patsubst $(DIR_SRC)%.c, $(DIR_SRC)%.o, $(FILE_SRC))

FLAG_LIB=-lnesl -lSDL2 -lSDL2main

.PHONY: all
all: build

.PHONY: build
build: $(FILE_BIN)

.PHONY: clean
clean:
	rm -rf $(FILE_OBJ)

$(DIR_SRC)%.o: $(DIR_SRC)%.c
	$(CC) $(FLAGS) -I$(DIR_INCLUDE) -c -o $@ $<

$(FILE_LIB): $(FILE_OBJ)
	ar rcs $@ $(FILE_OBJ)

$(FILE_BIN): $(FILE_LIB)
	$(CC) $(FLAGS) -L$(DIR_BUILD) $(FLAG_LIB) $(FILE_LIB) -o $@

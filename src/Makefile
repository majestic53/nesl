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

DIR_BUILD=../build/
DIR_INCLUDE=../include/
DIR_ROOT=./

FILE=nesl
FILE_BIN=$(DIR_BUILD)$(FILE)
FILE_PATCH=$(DIR_INCLUDE)common/define.h
FILES_OBJ=$(patsubst $(DIR_ROOT)%.c,$(DIR_ROOT)%.o,$(FILES_SRC))
FLAGS_PATCH="s/NESL_PATCH .*/NESL_PATCH 0x$(shell git rev-parse --short HEAD)/g"
FILES_SRC=$(shell find $(DIR_ROOT) -name '*.c')

FLAGS_INCLUDE=$(subst $(DIR_INCLUDE),-I$(DIR_INCLUDE),$(shell find $(DIR_INCLUDE) -maxdepth 2 -type d))
FLAGS_LIB=-lpthread -lSDL2

.PHONY: all
all: build

.PHONY: build
build: $(FILE_BIN)

.PHONY: clean
clean:
	@rm -rf $(DIR_BUILD)
	@rm -rf $(FILES_OBJ)

.PHONY: patch
patch:
	@sed -i $(FLAGS_PATCH) $(FILE_PATCH)

.PHONY: strip
strip:
	@strip $(DIR_BUILD)*

$(DIR_BUILD):
	@mkdir -p $@

$(DIR_ROOT)%.o: $(DIR_ROOT)%.c
	$(CC) $(FLAGS) $(FLAGS_INCLUDE) -c -o $@ $<

$(FILE_BIN): $(DIR_BUILD) $(FILES_OBJ)
	$(CC) $(FLAGS) $(FILES_OBJ) $(FLAGS_LIB) -o $@

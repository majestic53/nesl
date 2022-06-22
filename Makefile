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

DIR_DOCS=docs/
DIR_SRC=src/
DIR_TEST=test/

FLAGS=-march=native\ -mtune=native\ -std=c11\ -Wall\ -Werror
FLAGS_DEBUG=FLAGS=$(FLAGS)\ -DDEBUG\ -g
FLAGS_RELEASE=FLAGS=$(FLAGS)\ -O3
FLAGS_MAKE=--no-print-directory -C

MAX_PARALLEL=8

.PHONY: all
all: release

.PHONY: analyze
analyze:
	@cloc .
	@cppcheck --enable=all --std=c11 --suppress=missingIncludeSystem .

.PHONY: docs
docs:
	@rm -rf $(DIR_DOCS)html
	@doxygen $(DIR_DOCS)Doxyfile

.PHONY: debug
debug: clean
	@make $(FLAGS_MAKE) $(DIR_SRC) build -j${MAX_PARALLEL} $(FLAGS_DEBUG)

.PHONY: release
release: clean
	@make $(FLAGS_MAKE) $(DIR_SRC) patch
	@make $(FLAGS_MAKE) $(DIR_SRC) build -j${MAX_PARALLEL} $(FLAGS_RELEASE)
	@make $(FLAGS_MAKE) $(DIR_SRC) strip

.PHONY: test
test: test-release

.PHONY: test-debug
test-debug: clean
	@make $(FLAGS_MAKE) $(DIR_TEST) build $(FLAGS_DEBUG)
	@make $(FLAGS_MAKE) $(DIR_TEST) run

.PHONY: test-release
test-release: clean
	@make $(FLAGS_MAKE) $(DIR_TEST) build $(FLAGS_RELEASE)
	@make $(FLAGS_MAKE) $(DIR_TEST) run

.PHONY: clean
clean:
	@make $(FLAGS_MAKE) $(DIR_SRC) clean
	@make $(FLAGS_MAKE) $(DIR_TEST) clean
	@rm -rf $(DIR_DOCS)html

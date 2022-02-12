# NESL

[![License: MIT](https://shields.io/badge/license-MIT-blue.svg?style=flat)](https://github.com/majestic53/nesl/blob/master/LICENSE) [![Build Status](https://github.com/majestic53/nesl/workflows/Build/badge.svg)](https://github.com/majestic53/nesl/actions/workflows/build.yml)

A lightweight NES emulator, written in C.

## Features

__NOTE__: This project is a currently a work-in-progress. Some features may not be completed or may be buggy.

### Completed

`TBD`

### Work-in-progress

`TBD`

## Latest release

The latest release can be found here: `TBD`

## Building from source

__NOTE__: This project is tested under GNU/Linux, using the GCC C compiler. Some modifications will be required to build with a different OS/compiler.

### Preparing to build

The following libraries are required to build this project:
* [Simple Display Layer](https://www.libsdl.org/)

#### On Debian-based systems

Install the libraries using apt:

```
sudo apt-get install libsdl2-dev
```

#### On Arch-based systems

Install the libraries using pacman:

```
sudo pacman -S sdl2
```

### Cloning the project

Clone the project from the repo:

```
git clone https://github.com/majestic53/nesl
```

### Compiling the project

Navigate into the project directory and run the make command:

```
cd nesl && make
```

## Interface

Launch from the projects `build/` directory:

```
nesl [options] file
```

### Options

The following options are available:

|Option|Description             |
|:-----|:-----------------------|
|-f    |Set window fullscreen   |
|-h    |Show help information   |
|-l    |Set linear scaling      |
|-s    |Set window scaling      |
|-v    |Show version information|

#### Examples

To launch with a file, run the following command:

```
nesl file
```

To launch in fullscreen mode, run the following command:

```
nesl -f file
```

To launch using nearest scaling (1x-4x), run the following command:

```
nesl -s [1-4] file
```

To launch using linear scaling (1x-4x), run the following command:

```
nesl -ls [1-4] file
```

### Keybindings

The following keybindings are available:

#### Console

|Button    |Keyboard|
|:---------|:-------|
|Fullscreen|F11     |
|Reset     |R       |

#### Controllers

|Button |Controller 1|Controller 2|
|:------|:-----------|:-----------|
|A      |L           |N/A         |
|B      |K           |N/A         |
|Select |C           |N/A         |
|Start  |Space       |N/A         |
|Up     |W           |N/A         |
|Down   |S           |N/A         |
|Left   |A           |N/A         |
|Right  |D           |N/A         |

## License

Copyright (C) 2022 David Jolly

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

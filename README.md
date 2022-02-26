# NESL

[![License: MIT](https://shields.io/badge/license-MIT-blue.svg?style=flat)](https://github.com/majestic53/nesl/blob/master/LICENSE) [![Build Status](https://github.com/majestic53/nesl/workflows/Build/badge.svg)](https://github.com/majestic53/nesl/actions/workflows/build.yml) [![Test Status](https://github.com/majestic53/nesl/workflows/Test/badge.svg)](https://github.com/majestic53/nesl/actions/workflows/test.yml)

A lightweight NES emulator, written in C.

![Demo](https://github.com/majestic53/nesl/blob/master/docs/demo.png)

## Features

__NOTE__: This project is a currently a work-in-progress. Some features may not be completed or may be buggy.

### Completed

* Support for all official processor instructions.
* Support for common mapper types.

### Work-in-progress

* Support for audio playback.

## Latest release

The latest release can be found here: `TBD`

## Building from source

__NOTE__: This project is tested under GNU/Linux, using the GCC compiler. Some modifications will be required to build with a different OS/compiler.

### Preparing to build

The following libraries are required to build this project:
* [SDL2 (Simple DirectMedia Layer)](https://github.com/libsdl-org/SDL)

#### On Debian-based systems

Install the libraries using apt:

```bash
sudo apt-get install libsdl2-dev
```

#### On Arch-based systems

Install the libraries using pacman:

```bash
sudo pacman -S sdl2
```

### Cloning the project

Clone the project from the repo:

```bash
git clone https://github.com/majestic53/nesl
```

### Building the project

Navigate into the project directory and run make to build the binary:

```bash
cd nesl && make
```

## Using the binary

Launch the binary from `build/`:

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

To launch the binary with a file, run the following command:

```bash
nesl file
```

To launch the binary in fullscreen mode, run the following command:

```bash
nesl -f file
```

To launch the binary with nearest-scaling (1x-4x), run the following command:

```bash
nesl -s [1-4] file
```

To launch the binary with linear-scaling (1x-4x), run the following command:

```bash
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

### Mapper Support

The following mappers are supported:

|# |Mapper                                                            |Description   |
|:-|:-----------------------------------------------------------------|:-------------|
|0 |[NROM](https://wiki.nesdev.org/w/index.php?title=INES_Mapper_000) |No mapper     |
|1 |[MMC1](https://wiki.nesdev.org/w/index.php?title=INES_Mapper_001) |Nintendo ASIC |
|2 |[UxROM](https://wiki.nesdev.org/w/index.php?title=INES_Mapper_002)|Nintendo board|
|3 |[CNROM](https://wiki.nesdev.org/w/index.php?title=INES_Mapper_003)|Nintendo board|
|4 |[MMC3](https://wiki.nesdev.org/w/index.php?title=INES_Mapper_004) |Nintendo ASIC |
|30|[UNROM](https://wiki.nesdev.org/w/index.php?title=INES_Mapper_030)|Homebrew board|
|66|[GxROM](https://wiki.nesdev.org/w/index.php?title=INES_Mapper_066)|Nintendo board|

## License

Copyright (C) 2022 David Jolly. Released under the [MIT License](https://opensource.org/licenses/MIT).

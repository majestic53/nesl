# NESL (NES-Lite)

[![License: MIT](https://shields.io/badge/license-MIT-blue.svg?style=flat)](LICENSE.md) [![Build Status](https://github.com/majestic53/nesl/workflows/Build/badge.svg)](https://github.com/majestic53/nesl/actions/workflows/build.yml) [![Test Status](https://github.com/majestic53/nesl/workflows/Test/badge.svg)](https://github.com/majestic53/nesl/actions/workflows/test.yml)

### A lightweight NES emulator, written in C.

This project emulates the original NES hardware with enough accuracy to run most common games, while being performant and natively compilable on Linux, MacOS and Windows.

__NOTE__: This project is a still a work-in-progress. Some features may be incomplete or may contain bugs. See how to contribute and file issues [here](CONTRIBUTING.md#places-to-contribute).

<p align="center">
    <img src=docs/demo0.png><img src=docs/demo1.png>
    <br/>
    <img src=docs/demo2.png><img src=docs/demo3.png>
    <br/>
    <img src=docs/demo4.png><img src=docs/demo5.png>
</p>

## Features

### Completed

* Support for all official processor instructions.
* Support for common mapper types.
* Support for zapper (light-gun).

### Work-in-progress

* Support for audio synthesis.

## Latest release

The latest release can be found here: `TBD`

## Building from source

__NOTE__: This project has been tested under Linux, using the GCC compiler. Some modifications may be required to build with a different OS/compiler.

### Preparing to build

The following libraries are required to build this project:
* [SDL2 (Simple DirectMedia Layer)](https://wikipedia.org/wiki/Simple_DirectMedia_Layer)

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
|-h    |Show help information   |
|-l    |Set linear scaling      |
|-s    |Set window scaling      |
|-v    |Show version information|

#### Examples

To launch the binary with a file, run the following command:

```bash
nesl file
```

To launch the binary with nearest-scaling (1x-4x), run the following command:

```bash
nesl -s [1-8] file
```

To launch the binary with linear-scaling (1x-4x), run the following command:

```bash
nesl -ls [1-8] file
```

### Keybindings

The following keybindings are available:

#### General

|Button    |Keyboard|
|:---------|:-------|
|Reset     |R       |

#### Controller

|Button |Keyboard|
|:------|:-------|
|A      |L       |
|B      |K       |
|Select |C       |
|Start  |Space   |
|Up     |W       |
|Down   |S       |
|Left   |A       |
|Right  |D       |

#### Zapper (Light-gun)

|Button |Mouse|
|:------|:----|
|Trigger|Left |

### Mapper support

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

Copyright (C) 2022 David Jolly. Released under the [MIT License](LICENSE.md).

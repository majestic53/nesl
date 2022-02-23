# How to Contribute

## Getting Started

__NOTE__: This project is tested under GNU/Linux, using the GCC compiler. Some modifications will be required to build with a different OS/compiler.

### How to Setup Your Environment

The following libraries are required to build this project:
* [SDL2 (Simple DirectMedia Layer)](https://github.com/libsdl-org/SDL)

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

### Building the project

Navigate into the project directory and run make to build the binary:

```
cd nesl && make
```

### Running the tests

Navigate into the project directory and run make to build/run the tests:

```
cd nesl && make test
```

## Project Architecture

This project is built around a common bus, allowing the various subsystems to communicate with each other:

![Architecture](https://github.com/majestic53/nesl/blob/master/docs/arch.png "Architecture")

### Adding New Files

* All subsystems should conform to the model above, and avoid calling each other directly. Instead, they should communicate through the common bus.
* Any subsystem specific source files should be placed into a subdirectory, under `src/system`.

### File Layout

|Directory                                                                            |Description                          |
|:------------------------------------------------------------------------------------|:------------------------------------|
|[src/common](https://github.com/majestic53/nesl/tree/master/src/common)              |Common source files                  |
|[src/service](https://github.com/majestic53/nesl/tree/master/src/service)            |Service source files (SDL by default)|
|[src/system](https://github.com/majestic53/nesl/tree/master/src/system)              |Subsystem source files               |
|[src/system/audio](https://github.com/majestic53/nesl/tree/master/src/system/audio)  |Audio-specific source files          |
|[src/system/mapper](https://github.com/majestic53/nesl/tree/master/src/system/mapper)|Mapper-specific source files         |

### Coding Style

Functions should be in the format:

```c
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

...

int NESL_SubsystemXxx(int arg0, int arg1, ...)
{
    int result = NESL_SUCCESS;

    if(arg0 == 0) {
        result = NESL_FAILURE;
        goto exit;
    }

    ...

exit:
    return result;
}

...

#ifdef __cplusplus
}
#endif /* __cplusplus */
```

## Places to Contribute

### Bug Fixes to Existing Code

This project is a currently a work-in-progress. Some features may not be completed or may be buggy. Any bug fixes in existing code would be greatly appreciated.

### Adding Additional Mapper Support

This project only implements a few of the avaliable mappers the original NES supported. Adding more mappers will increase the number of games supported.
* For a full list of mappers, see the [NESDEV wiki](https://wiki.nesdev.org/w/index.php?title=Mapper).

### Adding Undocumented Instruction Support

This project only implements the official documented instructions in the processor. A few games take advantage of undocumented instructions, and are not currently supported.

## Issues

If you encounter any issues while using this project, please report it via [Issues](https://github.com/majestic53/nesl/issues).

## Pull Requests

If you have a change ready to be submitted, please create a PR request via [Pulls](https://github.com/majestic53/nesl/pulls)

## Contributors

Special thanks to all who contribute to this project!

<a href="https://github.com/majestic53/nesl/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=majestic53/nesl" />
</a>

Made with [contrib.rocks](https://contrib.rocks).

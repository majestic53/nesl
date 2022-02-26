# How to Contribute

## Getting Started

See the build steps in [`README.md`](https://github.com/majestic53/nesl/blob/master/README.md#building-from-source)

## Project Architecture

This project is built around a common bus, allowing the various subsystems to communicate with each other:

![Architecture](https://github.com/majestic53/nesl/blob/master/docs/arch.png "Architecture")

### Adding new files

* All subsystems should conform to the model above, and avoid calling each other directly. Instead, they should communicate through the common bus.
* Any subsystem specific source files should be placed into a subdirectory, under [`src/system`](https://github.com/majestic53/nesl/tree/master/src/system).

### File layout

|Directory                                                                              |Description                          |
|:--------------------------------------------------------------------------------------|:------------------------------------|
|[`src/common`](https://github.com/majestic53/nesl/tree/master/src/common)              |Common source files                  |
|[`src/service`](https://github.com/majestic53/nesl/tree/master/src/service)            |Service source files (SDL by default)|
|[`src/system`](https://github.com/majestic53/nesl/tree/master/src/system)              |Subsystem source files               |
|[`src/system/audio`](https://github.com/majestic53/nesl/tree/master/src/system/audio)  |Audio-specific source files          |
|[`src/system/mapper`](https://github.com/majestic53/nesl/tree/master/src/system/mapper)|Mapper-specific source files         |

### Coding style

Code should be in the format:

```c
typedef struct {
    int val_0;
    int val_1;
    ...
} nesl_xxx_t;

...

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

...

int NESL_XxxYyy(nesl_xxx_t *arg_0, int arg_1, ...)
{
    int result = NESL_SUCCESS;

    if(arg_0 == NULL) {
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

### Bug-fixes in existing code

This project is a currently a work-in-progress. Some features may not be completed or may be buggy. Any bug fixes in existing code would be greatly appreciated.

### Adding additional mapper support

This project only implements a few of the common mappers the original NES supported. Adding more mappers will increase the number of games supported.
* For a full list of possible mappers, see the [NESDEV wiki](https://wiki.nesdev.org/w/index.php?title=Mapper).

### Adding undocumented instruction support

This project only implements the official documented instructions in the processor. A few games take advantage of undocumented instructions, and are not currently supported.
* For a list of undocumented instructions, see the [NESDEV wiki](https://wiki.nesdev.org/w/index.php?title=CPU_unofficial_opcodes).

## Issues

If you encounter any issues while using this project, please report them via [Issues](https://github.com/majestic53/nesl/issues).

## Pull Requests

If you have a change ready to be submitted, please create a pull request via [Pulls](https://github.com/majestic53/nesl/pulls)

## Contributors

Special thanks to all who contribute to this project!

<a href="https://github.com/majestic53/nesl/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=majestic53/nesl" />
</a>

Made with [contrib.rocks](https://contrib.rocks).

# How to Contribute

## Getting Started

* See the build steps in the readme [here](README.md#building-from-source).
* See the architectural notes in the readme [here](src/README.md).

## Places to Contribute

### Bug-fixes in existing code

`NESL` is a currently a work-in-progress. Some features may not be completed or may be buggy. Any bug fixes in existing code would be greatly appreciated.

### Adding audio support

`NESL` only implements a skeleton for the audio subsystem. This subsystem could be implemented to enable audio emulation.

### Adding additional mapper support

`NESL` only implements a few of the common mappers the original NES supported. Adding more mappers will increase the number of games supported.
* For a full list of possible mappers, see the [NESDEV wiki](https://wiki.nesdev.org/w/index.php?title=Mapper).

### Adding undocumented instruction support

`NESL` only implements the official documented instructions in the processor. A few games take advantage of undocumented instructions, and are not currently supported.
* For a list of undocumented instructions, see the [NESDEV wiki](https://wiki.nesdev.org/w/index.php?title=CPU_unofficial_opcodes).

## Issues

If you encounter any issues while using `NESL`, please report them via [Issues](https://github.com/majestic53/nesl/issues).

## Pull Requests

If you have a change ready to be submitted, please create a pull request via [Pulls](https://github.com/majestic53/nesl/pulls)

## Contributors

Special thanks to all who contribute to `NESL`!

<a href="https://github.com/majestic53/nesl/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=majestic53/nesl" />
</a>

Made with [contrib.rocks](https://contrib.rocks).

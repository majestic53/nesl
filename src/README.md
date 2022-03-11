# Project Architecture

This project is built around a common bus, allowing the various subsystems to communicate with each other:

![Architecture](https://github.com/majestic53/nesl/blob/master/docs/arch.png "Architecture")

## File layout

* Subsystem specific source files are placed into a subdirectory, under [`src/system`](https://github.com/majestic53/nesl/tree/master/src/system).

|Directory                                                                              |Description                          |
|:--------------------------------------------------------------------------------------|:------------------------------------|
|[`src/common`](https://github.com/majestic53/nesl/tree/master/src/common)              |Common source files                  |
|[`src/service`](https://github.com/majestic53/nesl/tree/master/src/service)            |Service source files (SDL by default)|
|[`src/system`](https://github.com/majestic53/nesl/tree/master/src/system)              |Subsystem source files               |
|[`src/system/audio`](https://github.com/majestic53/nesl/tree/master/src/system/audio)  |Audio-specific source files          |
|[`src/system/mapper`](https://github.com/majestic53/nesl/tree/master/src/system/mapper)|Mapper-specific source files         |

## Coding style

This project uses the following coding style:

### Enumerations

```c
/**
 * ...
 * Doxygen comment
 * ...
 */
typedef enum {
    XXX_A = 0,          /*< Doxygen comment */
    XXX_B,              /*< Doxygen comment */
    ...
} nesl_xxx_e;
```

### Structures

```c
/**
 * ...
 * Doxygen comment
 * ...
 */
typedef struct {
    nesl_xxx_e val_0;   /*< Doxygen comment */
    int val_1;          /*< Doxygen comment */
    ...
} nesl_xxx_t;
```

### Functions

```c
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

...

/**
 * ...
 * Doxygen comment
 * ...
 */
int nesl_xxx_yyy(nesl_xxx_t *arg_0, nesl_xxx_e arg_1, ...)
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

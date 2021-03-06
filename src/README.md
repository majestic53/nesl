# Project Architecture

This project contains a series of subsystems, each representing one component of the phyiscal hardware in the NES. The subsystems each communicate with each other and are driven by a central bus and service. The bus and service interface to the outside world for configuration and IO. The following diagram gives a high-level idea of how all the components fit together:

![Architecture](../docs/arch.png "Architecture")

## File layout

Subsystem source files can be found under the following directories:

|Directory                           |Description                          |
|:-----------------------------------|:------------------------------------|
|[`src/common`](common)              |Common source files                  |
|[`src/service`](service)            |Service source files (SDL by default)|
|[`src/system`](system)              |Subsystem source files               |
|[`src/system/audio`](system/audio)  |Audio-specific source files          |
|[`src/system/mapper`](system/mapper)|Mapper-specific source files         |

## Testing

Each subsystem has unit-tests, found under [`test`](../test)

## Coding style

The source code generally conforms to the following coding standard:

### Enumerations

```c
/*!
 * @enum nesl_xxx_e
 * @brief ...
 */
typedef enum {
    XXX_A = 0,          /*!< Doxygen comment */
    XXX_B,              /*!< Doxygen comment */
    ...
} nesl_xxx_e;
```

### Structures

```c
/*!
 * @struct nesl_xxx_t
 * @brief ...
 */
typedef struct {
    nesl_xxx_e val_0;   /*!< Doxygen comment */
    int val_1;          /*!< Doxygen comment */
    ...
} nesl_xxx_t;
```

### Functions

```c
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

...

/*!
 * @brief ...
 * @param[in,out] arg_0 ...
 * @param[in] arg_1 ...
 * @return ...
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

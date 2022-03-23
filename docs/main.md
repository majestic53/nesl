This project emulates the original NES hardware with enough accuracy to run most common games, while being performant and natively compilable on Linux, MacOS and Windows.

### Project Architecture

This project is built around a common bus, allowing the various subsystems to communicate with each other:

![](arch.png)

### File layout

Subsystem specific source files are placed into a subdirectory, under `src/system`.

|Directory          |Description                          |
|:------------------|:------------------------------------|
|`src/common`       |Common source files                  |
|`src/service`      |Service source files (SDL by default)|
|`src/system`       |Subsystem source files               |
|`src/system/audio` |Audio-specific source files          |
|`src/system/mapper`|Mapper-specific source files         |

### Coding style

This project uses the following coding style:

#### Enumerations

    /*!
     * @enum nesl_xxx_e
     * @brief ...
     */
    typedef enum {
        XXX_A = 0,          /*!< Doxygen comment */
        XXX_B,              /*!< Doxygen comment */
        ...
    } nesl_xxx_e;

#### Structures

    /*!
     * @struct nesl_xxx_t
     * @brief ...
     */
    typedef struct {
        nesl_xxx_e val_0;   /*!< Doxygen comment */
        int val_1;          /*!< Doxygen comment */
        ...
    } nesl_xxx_t;

#### Functions

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

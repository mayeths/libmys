# LibMYS

Mayeths' library (wow@mayeths.com).

### `asset`

Useful and interesting files.

### `bin`

Executables (commonly writtern in Shell Script and Python).

### `external`

Source code of external libraries.

### `include`

C/C++ headers for self use.

- **mys**: **M**a**y**eth**s**' Basic Header Library
    - ***.h**: Headers that work under C99
    - ***.hpp**: Headers that work under C++11
- **mys0**: Mayeths' Machine Specific Library (and won't be upload to git)
- **mys3**: Mayeths' **3**rd-Party Library
- **mysg**: Mayeths' **G**raphics Library
- **mysp**: Mayeths' **P**erformance Analysis Library
- **myss**: Mayeths' **S**parse Solver Library


Should we force MYS_IMPL ?
```c
// in main.c
#define MYS_IMPL
#include <mys.h>
// in other files
#include <mys.h>
```

# LibMYS

Mayeths' library (wow@mayeths.com).

- **mys**: **M**a**y**eth**s**' Basic Header Library
    - ***.h**: Headers that work under C99
    - ***.hpp**: Headers that work under C++11
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

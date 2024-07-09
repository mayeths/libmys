# Libmys: Mayeths' Library

Mayeths' library for daily development (wow@mayeths.com).

I've put in a lot of effort to make this code portable, especially the C/C++ core in `include/mys`, which I use every day. It's designed to run across various systems and architectures, though there's no warranty. I use `-Wall -Wextra -Werror` to ensure usability, but many functions were written from scratch as needed. Bugs were fixed as they came up, meaning I only changed the code when I noticed something off. My primary development environment has shifted to macOS, with Linux for most remote development. This means the code has been thoroughly tested on POSIX-compliant systems.

Libmys is licensed under the MIT license, including `include/mys`. Third-party code (`include/mys3` and any files indicating their original source) retains its original license, and users should check their licenses before using Libmys in a commercial environment.

### The history of libmys

I originally created this library to avoid repetitive and messy coding. As it grew, I realized it could include more than just C/C++ files. Now, it also houses config files, shell scripts, Python modules, and more. So, it has evolved beyond being just a simple C/C++ library.

The first commit of libmys had just two header files, one for a C project and another for a C++ project. I loved copying and pasting them into different projects, using many static variables and functions. Over time, it's grown into a substantial repository, packed with both my code and code sourced from the internet.

### Setting up libmys

```bash
# Clone libmys
git clone https://github.com/mayeths/libmys.git
# Enjoy libmys
export MYS_DIR=~/project/libmys
export MYS_MODDIR=~/module
source "$MYS_DIR/etc/profile"
# Rsync libmys
cd libmys
rsync --filter=':- .gitignore' -av ./ REMOTE:~/project/libmys/
```

### C language

#### Example

See `test/basic-gcc.c`

```c
//// main.c
#include <mpi.h>
#define MYS_IMPL
#include <mys.h>

extern void hello_again();

int main() {
    MPI_Init(NULL, NULL);
    DLOG(0, "Hello world from process 0! (debug message)");
    hello_again();
    MPI_Finalize();
    return 0;
}

//// sub.c
#include <mys.h>

void hello_again() {
    ILOG_ORDERED("Hello world from all MPI processes!");
}
```

```bash
$ mpicc -I${MYS_DIR}/include main.c sub.c
$ mpirun -n 2 ./a.out
[D::000 main.c:009] Hello world from process 0! (debug message)
[I::000 sub.c:006] Hello world from all MPI processes!
[I::001 sub.c:006] Hello world from all MPI processes!
```


#### Macros

`#define MYS_IMPL` or `#define MYS_IMPL_LOCAL` macros once to expand the main implementation body of libmys.

By default, libmys is configured for the MPI environment that uses `mpicc` or `mpic++` as compilers. If users use non-MPI compilers like `gcc` and `g++`, `#define MYS_NO_MPI` alongside `#include <mys.h>`.

| Macro | Description |
|-------|-------------|
| MYS_IMPL | Define libmys functions and variables in this file. |
| MYS_IMPL_LOCAL | Define libmys functions and variables in this file with hidden visibility. In this case, libmys in dynamic libraries and executables operate independently without interference. |
| MYS_NO_MPI | Disable MPI support. |

# Libmys: Mayeths' Library

This is Mayeths' library for everyday coding (wow@mayeths.com, huanghp22@mails.tsinghua.edu.cn).

I've put a lot of effort into making this code easy to use on different systems. The main C/C++ part is in `include/mys`, which I use every day. It's designed to work on POSIX systems (Linux, macOS) and different CPUs (x86_64 and arm). Still, I can't promise everything works perfectly. I use strict compile options `-Wall -Wextra -Werror` and test it on Linux and macOS through GitHub Actions. I wrote many of the functions myself and fixed bugs as I found them, so not all parts were tested ahead of time. I mostly code on macOS and use Linux when working remotely, so the code works well on both systems.

[![(Github Action) Compile and test libmys/include/mys](https://github.com/mayeths/libmys/actions/workflows/test.yml/badge.svg)](https://github.com/mayeths/libmys/actions/workflows/test.yml)

Libmys uses the MIT license, including everything in `include/mys`. Any third-party code (like `include/mys3` or files that show where they came from) keeps its original license. If you want to use Libmys in commercial projects, please check the licenses first.

### The story behind libmys

I first made this library to avoid writing the same messy code again and again. As time passed, I saw that it could be more than just C/C++ files. Now it also includes config files, shell scripts, Python code, and more. So it's not just a simple header-only library anymore.

In the beginning, libmys only had two header files (one for C, and one for C++). I liked copying and pasting them into different projects, and they used a lot of static variables and functions. Over time, the library got much bigger, filled with both my own code and useful code I found online.

The design philosophy comes from [stb](https://github.com/nothings/stb), a single-file public domain libraries for C/C++. stb use macro `STB_IMAGE_IMPLEMENTATION` to actually enable the function definitions. I use `MYS_IMPL` in a similar way.

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

See examples in `test/`

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

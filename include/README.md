### Libmys C/C++ library

I initially created this library to avoid repeating some messy coding. However, as the library grew, I realized it could house not only C/C++ files but also any resources that could assist me in my daily development, such as config files, shell scripts, Python modules, and more. As a result, this library has evolved beyond being a simple C/C++ library.

The first commit of Libmys included just two header files, one for a C project and another for a C++ project. I enjoyed copying and pasting them into various projects I wanted to integrate, utilizing many static variables and functions. Unfortunately, due to an incomplete commit history, I can't pinpoint the oldest code. Despite this, the first commit already contained 15 files and 1959 lines of code. Over time, it has grown into a substantial repository, filled with both my own code and code sourced from the internet.

I have made significant efforts to ensure the portability of these codes (especially those in `include/mys`, the core of this project) across many systems and architectures, though without any warranty. I use `-Wall -Wextra -Werror` to maintain usability, but many functions related to the OS and file systems were written from scratch on a case-by-case basis. Bugs were often fixed as they were encountered, meaning I would only modify the code upon noticing unexpected results. My primary development environment has shifted to MacOS, with Linux being used for most remote development scenarios. Consequently, these codes have been extensively tested on POSIX-compliant systems, which has been a satisfying experience.

### Coding Style

- Static functions that wants to be used by users should be placed in headers with `MYS_STATIC`, e.g., math.h, instead of math.c.
- Static functions that used internally should be placed in implementation files with `MYS_STATIC`.
- Standard and external headers should be included wherever they are needed, for instance in implementation files.


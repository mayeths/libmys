### Libmys Coding Style

- Static functions that are intended to be used by users should be placed in headers with `MYS_STATIC`, e.g., math.h, instead of math.c.
- Static functions that are used internally should be placed in implementation files with `MYS_STATIC`.
- Standard and external headers should be included wherever they are needed, for instance, in implementation files.

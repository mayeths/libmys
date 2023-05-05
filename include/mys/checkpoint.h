#pragma once

#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdarg.h>

#include "os.h"
#include "hrtime.h"
#include "log.h"

MYS_API void mys_checkpoint_init();
MYS_API void mys_checkpoint_reset();
MYS_API void mys_checkpoint(const char *name_format, ...);
MYS_API int mys_checkpoint_dump(const char *file_format, ...);

#define CHECKPOINT(name_format, ...) mys_checkpoint(name_format, ##__VA_ARGS__)

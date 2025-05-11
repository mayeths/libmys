/* 
 * Copyright (c) 2025 Haopeng Huang - All Rights Reserved
 * 
 * Licensed under the MIT License. You may use, distribute,
 * and modify this code under the terms of the MIT license.
 * You should have received a copy of the MIT license along
 * with this file. If not, see:
 * 
 * https://opensource.org/licenses/MIT
 */
#include "../backtrace.h"
#include "../memory.h"
#include "../pmparser.h"
#include "../os.h"

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

MYS_PUBLIC int mys_backtrace(void **addrs, int max_depth)
{
    if (addrs == NULL || max_depth <= 0) {
        return 0;
    }
    int depth = backtrace(addrs, max_depth);
    for (int i = 0; i < depth - 1; ++i) {
        addrs[i] = addrs[i + 1];
    }
    return depth - 1;
}

MYS_PUBLIC void mys_backtrace_source(void *addr, char *source, size_t max_size)
{
    const char *self_exe = mys_procname();
    struct stat self_st;
    stat(self_exe, &self_st);

    const char *target = self_exe;
    void *relative = addr;
    mys_procmaps_t *self = mys_pmparser_self();
    mys_procmap_t *map = self->head;
    while (map) {
        if (addr >= map->addr_start && addr < map->addr_end) {
            struct stat st;
            if (stat(map->pathname, &st) == 0) {
                bool is_self_exe = (st.st_ino == self_st.st_ino && st.st_dev == self_st.st_dev);
                if (!is_self_exe) {
                    target = map->pathname;
                    relative = (void *)((uintptr_t)addr - (uintptr_t)map->addr_start);
                }
            }
            break;
        }
        map = map->next;
    }

    char bufcmd[1024];
    snprintf(bufcmd, sizeof(bufcmd), "addr2line -e %s %p", target, relative);
    mys_prun_t run = mys_prun_create(bufcmd, source, max_size, NULL, 0);
    mys_prun_destroy(&run);
}

MYS_PUBLIC void mys_backtrace_symbol(void *addr, char *symbol, size_t max_size)
{
    // use backtrace_symbols()
    char **bsyms = backtrace_symbols(&addr, 1);
    strncpy(symbol, bsyms[0], max_size - 1);
    symbol[max_size - 1] = '\0';
    free(bsyms);
}

/* 
 * Copyright (c) 2024 Haopeng Huang - All Rights Reserved
 * 
 * Licensed under the MIT License. You may use, distribute,
 * and modify this code under the terms of the MIT license.
 * You should have received a copy of the MIT license along
 * with this file. If not, see:
 * 
 * https://opensource.org/licenses/MIT
 */
#include "_private.h"
#include "../backtrace.h"
#include "../memory.h"
#include "../os.h"

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

MYS_PUBLIC int mys_backtrace(mys_backtrace_t *buffer, int max_depth) {
    void *addresses[128];
    int depth = backtrace(addresses, max_depth);
    // char **symbols = backtrace_symbols(addresses, depth);

    // if (symbols == NULL) {
    //     return -1;  // Failed to get symbols
    // }

    for (int i = 0; i < depth; ++i) {
        buffer[i].pc = addresses[i];
        const char *self_exe = mys_procname();
        char bufcmd[256];
        snprintf(bufcmd, sizeof(bufcmd), "addr2line -e %s %p", self_exe, addresses[i]);
        mys_prun_t run = mys_prun_create(bufcmd, buffer[i].source, sizeof(buffer[i].source), NULL, 0);
        mys_prun_destroy(&run);

        // // Extract function name and source file information
        // Dl_info dlinfo;
        // if (dladdr(addresses[i], &dlinfo) && dlinfo.dli_sname) {
        //     // Demangle the function name if possible
        //     int status = 0;
        //     char *demangled = abi::__cxa_demangle(dlinfo.dli_sname, NULL, NULL, &status);
        //     if (status == 0 && demangled != NULL) {
        //         strncpy(buffer[i].symbol, demangled, sizeof(buffer[i].symbol) - 1);
        //         free(demangled);
        //     } else {
        //         strncpy(buffer[i].symbol, dlinfo.dli_sname, sizeof(buffer[i].symbol) - 1);
        //     }
        //     buffer[i].symbol[sizeof(buffer[i].symbol) - 1] = '\0';
        // } else {
        //     strncpy(buffer[i].symbol, symbols[i], sizeof(buffer[i].symbol) - 1);
        //     buffer[i].symbol[sizeof(buffer[i].symbol) - 1] = '\0';
        // }

        // if (dlinfo.dli_fname) {
        //     strncpy(buffer[i].source_file, dlinfo.dli_fname, sizeof(buffer[i].source_file) - 1);
        //     buffer[i].source_file[sizeof(buffer[i].source_file) - 1] = '\0';
        // } else {
        //     buffer[i].source_file[0] = '\0';
        // }

        // buffer[i].function = dlinfo.dli_saddr;

        // Source line is typically not available through dladdr, so we set it to -1
        // buffer[i].source_line = -1;
    }

    // free(symbols);
    return depth;
}

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
#include "../memory.h"
#include "../format.h"

typedef struct mys_fmtrun_t {
    mys_fmt_pass pass_fn;
    char *pass_spec;
} mys_fmtrun_t;

typedef struct mys_fmtex_t {
    mys_fmtrun_t *runs;
    size_t nrun;
} mys_fmtex_t;

#define MYS_FORMATER_MAX_PASSES 8

typedef struct mys_fmter_t {
    size_t npass;
    char *pass_names[MYS_FORMATER_MAX_PASSES];
    mys_fmt_pass pass_fns[MYS_FORMATER_MAX_PASSES];
} mys_fmter_t;


///// fmter

MYS_PUBLIC mys_fmter_t *mys_fmter_create()
{
    mys_fmter_t *fmter = (mys_fmter_t *)mys_malloc2(mys_arena_format, sizeof(mys_fmter_t));
    fmter->npass = 0;
    return fmter;
}

MYS_PUBLIC void mys_fmter_destroy(mys_fmter_t **fmter)
{
    if (fmter && *fmter) {
        for (size_t i = 0; i < (*fmter)->npass; ++i) {
            mys_free2(mys_arena_format, (*fmter)->pass_names[i], strlen((*fmter)->pass_names[i]));
        }
        mys_free2(mys_arena_format, *fmter, sizeof(mys_fmter_t));
        *fmter = NULL;
    }
}

MYS_PUBLIC void mys_fmter_register_pass(mys_fmter_t *fmter, const char *pass_name, mys_fmt_pass pass_fn)
{
    if (fmter->npass < MYS_FORMATER_MAX_PASSES) {
        size_t len = (size_t)strlen(pass_name);
        fmter->pass_names[fmter->npass] = (char *)mys_malloc2(mys_arena_format, len + 1);
        strncpy(fmter->pass_names[fmter->npass], pass_name, len);
        fmter->pass_names[fmter->npass][len] = '\0';
        fmter->pass_fns[fmter->npass] = pass_fn;
        fmter->npass++;
    }
}

MYS_PUBLIC mys_fmtex_t *mys_fmter_compile(mys_fmter_t *fmter, const char *fmtex_str)
{
    mys_fmtex_t *fmtex = (mys_fmtex_t *)mys_malloc2(mys_arena_format, sizeof(mys_fmtex_t));
    fmtex->runs = NULL;
    fmtex->nrun = 0;

    const char *ptr = fmtex_str;
    // printf("compiling %s", fmtex_str);
    while (*ptr != '\0') {
        const char *left_brace = strchr(ptr, '{');
        int direct_copy_len = 0;
        if (left_brace == NULL) {
            direct_copy_len = strlen(ptr);
        } else if (left_brace > ptr) {
            direct_copy_len = left_brace - ptr;
        } else {
            // A possible pass found
            assert(left_brace == ptr);
            const char *right_brace = strchr(ptr, '}');
            const char *next_left_brace = strchr(ptr + 1, '{');
            if (!right_brace) {
                // malformed format found
                direct_copy_len = strlen(ptr);
            } else if (next_left_brace && next_left_brace < right_brace) {
                // malformed format found
                direct_copy_len = next_left_brace - left_brace;
            } else {
                // Found a pass
                char pass_name[32];
                char pass_spec[32];
                const char *colon = strchr(ptr, ':');
                int name_len;
                int spec_len;
                if (colon && colon < right_brace) {
                    name_len = colon - ptr - 1;
                    spec_len = right_brace - colon - 1;
                } else {
                    name_len = right_brace - ptr - 1;
                    colon = ptr; // prevent strncpy failed
                    spec_len = 0;
                }
                strncpy(pass_name, ptr + 1, name_len);
                strncpy(pass_spec, colon + 1, spec_len);
                pass_name[name_len] = '\0';
                pass_spec[spec_len] = '\0';
                // printf("    found pass |%s|%s|\n", pass_name, pass_spec);

                mys_fmt_pass pass_fn = NULL;
                for (size_t i = 0; i < fmter->npass; ++i) {
                    if (strcmp(fmter->pass_names[i], pass_name) == 0) {
                        pass_fn = fmter->pass_fns[i];
                        break;
                    }
                }
                if (pass_fn) {
                    fmtex->runs = (mys_fmtrun_t *)mys_realloc2(mys_arena_format, fmtex->runs,
                        sizeof(mys_fmtrun_t) * (fmtex->nrun + 1), sizeof(mys_fmtrun_t) * (fmtex->nrun)
                    );
                    fmtex->runs = realloc(fmtex->runs, sizeof(mys_fmtrun_t) * (fmtex->nrun + 1));
                    fmtex->runs[fmtex->nrun].pass_fn = pass_fn;
                    fmtex->runs[fmtex->nrun].pass_spec = (char *)mys_malloc2(mys_arena_format, spec_len + 1);
                    strncpy(fmtex->runs[fmtex->nrun].pass_spec, pass_spec, spec_len);
                    fmtex->runs[fmtex->nrun].pass_spec[spec_len] = '\0';
                    fmtex->nrun++;
                    ptr = right_brace + 1;
                } else { // pass not found
                    /* solution 1: direct copy it */
                    // direct_copy_len = right_brace - left_brace + 1;
                    /* solution 2: skip it */
                    ptr = right_brace + 1;
                }
            }
        }

        if (direct_copy_len != 0) {
            fmtex->runs = (mys_fmtrun_t *)mys_realloc2(mys_arena_format, fmtex->runs,
                sizeof(mys_fmtrun_t) * (fmtex->nrun + 1), sizeof(mys_fmtrun_t) * (fmtex->nrun)
            );
            fmtex->runs[fmtex->nrun].pass_fn = NULL;
            fmtex->runs[fmtex->nrun].pass_spec = (char *)mys_malloc2(mys_arena_format, direct_copy_len + 1);
            strncpy(fmtex->runs[fmtex->nrun].pass_spec, ptr, direct_copy_len);
            fmtex->runs[fmtex->nrun].pass_spec[direct_copy_len] = '\0';
            fmtex->nrun++;
            ptr = ptr + direct_copy_len;
        }

    }

    return fmtex;
}

///// fmtex

MYS_PUBLIC mys_string_t *mys_fmtex_apply(mys_fmtex_t *fmtex, void *ctx)
{
    mys_string_t *buf = mys_string_create();

    for (size_t i = 0; i < fmtex->nrun; ++i) {
        mys_fmtrun_t *run = &fmtex->runs[i];
        if (run->pass_fn == NULL) {
            mys_string_append(buf, run->pass_spec);
        } else {
            run->pass_fn(buf, run->pass_spec, ctx);
        }
    }

    return buf;
}

MYS_PUBLIC void mys_fmtex_free(mys_fmtex_t **fmtex)
{
    if (*fmtex) {
        for (size_t i = 0; i < (*fmtex)->nrun; ++i) {
            mys_free2(mys_arena_format, (*fmtex)->runs[i].pass_spec, strlen((*fmtex)->runs[i].pass_spec));
        }
        mys_free2(mys_arena_format, (*fmtex)->runs, sizeof(mys_fmtrun_t) * (*fmtex)->nrun);
        mys_free2(mys_arena_format, *fmtex, sizeof(mys_fmtex_t));
        *fmtex = NULL;
    }
}

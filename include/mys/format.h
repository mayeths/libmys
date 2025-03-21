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
#pragma once

/*
 * mys_format: python-like string formatting system for customizable format specifiers.
 *
 * This module provides a 3-phase formatting system inspired by Python-style formatting:
 * 
 * 1. Define formatter and register format passes:
 *    Functions (format passes) that handle specific format specifiers are registered.
 * 
 * 2. Compile template string into a compiled format expression (fmtex):
 *    A format string is compiled into a series of instructions (fmtex), where each instruction
 *    corresponds to a pass function for handling format specifiers.
 * 
 * 3. Apply formatting:
 *    The compiled format expression is applied to a specific context (e.g., path, message, timestamp).
 */

#include "_config.h"
#include "macro.h"
#include "string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <time.h>
#include <stdbool.h>

typedef struct mys_fmter_t mys_fmter_t;
typedef struct mys_fmtex_t mys_fmtex_t;
typedef bool (*mys_fmt_pass)(mys_string_t *buf, const char *pass_spec, void *pass_ctx);

///// fmter

MYS_PUBLIC mys_fmter_t *mys_fmter_create();
MYS_PUBLIC void mys_fmter_destroy(mys_fmter_t **fmter);
/**
 * @brief Register a format pass with a formatter.
 *
 * This function registers a named pass that can be referenced in format strings.
 * Passes can handle specific formatting tasks (e.g., converting dates, formatting text).
 *
 * @param[in] fmter     The formatter object.
 * @param[in] pass_name The name of the pass, which will be used in format strings.
 * @param[in] pass_fn   The function that implements the pass.
 */
MYS_PUBLIC void mys_fmter_register_pass(mys_fmter_t *fmter, const char *pass_name, mys_fmt_pass pass_fn);
/**
 * @brief Compile a format expression into a reusable form.
 *
 * This function parses a format string and converts it into a compiled format expression (fmtex),
 * where each part of the format string corresponds to a pass that will be applied later.
 *
 * @param[in] fmter      The formatter object.
 * @param[in] fmtex_str  The format expression string.
 *
 * @return A pointer to a compiled format expression (`mys_fmtex_t`).
 */
MYS_PUBLIC mys_fmtex_t *mys_fmter_compile(mys_fmter_t *fmter, const char *fmtex_str);

///// fmtex

/**
 * @brief Apply a compiled format expression to a context.
 *
 * This function applies the compiled format expression (fmtex) to a given context,
 * executing the registered passes in the format string and generating the formatted output.
 *
 * @param[in] fmtex  The compiled format expression.
 * @param[in] ctx    The user-defined context that is passed to the format passes.
 *
 * @return A dynamically allocated string containing the formatted result.
 */
MYS_PUBLIC mys_string_t *mys_fmtex_apply(mys_fmtex_t *fmtex, void *ctx);
/**
 * @brief Free a compiled format expression.
 *
 * This function frees the memory associated with a compiled format expression.
 *
 * @param[in,out] fmtex A pointer to the compiled format expression to be freed.
 *                      This will be set to NULL after freeing.
 */
MYS_PUBLIC void mys_fmtex_free(mys_fmtex_t **fmtex);




/* gcc -I${MYS_DIR}/include -g test-formatter.c && valgrind ./a.out
#define MYS_IMPL
#define MYS_NO_MPI
#include <mys.h>

struct context_t {
    const char *path;
    const char *message;
    va_list message_args;
};

static bool fpass_asctime(mys_string_t *buf, const char *pass_spec, void *pass_ctx) {
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[256];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    size_t len = strftime(buffer, sizeof(buffer), pass_spec, timeinfo);
    if (len == 0)
        buffer[0] = '\0';

    mys_string_append_n(buf, buffer, len);
    return true;
}

static bool fpass_path(mys_string_t *buf, const char *pass_spec, void *pass_ctx) {
    struct context_t *ctx = (struct context_t *)pass_ctx;
    const char *full_path = ctx->path;
    const char *path = NULL;

    if (strlen(pass_spec) == 0)
        path = full_path;
    else if (strcmp(pass_spec, "l") == 0)
        path = full_path;
    else if (strcmp(pass_spec, "s") == 0)
        path = (strrchr(full_path, '/') ? strrchr(full_path, '/') + 1 : full_path);
    else
        return false;

    mys_string_append(buf, path);
    return true;
}

static bool fpass_message(mys_string_t *buf, const char *pass_spec, void *pass_ctx) {
    struct context_t *ctx = (struct context_t *)pass_ctx;

    va_list args_copy;
    va_copy(args_copy, ctx->message_args);
    int msg_len = vsnprintf(NULL, 0, ctx->message, args_copy);
    va_end(args_copy);

    char *msg_buffer = malloc(msg_len + 1);
    vsnprintf(msg_buffer, msg_len + 1, ctx->message, ctx->message_args);
    mys_string_append_n(buf, msg_buffer, msg_len);
    free(msg_buffer);

    return true;
}

mys_string_t *trylog(mys_fmtex_t *fmtex, const char *message, ...)
{
    struct context_t ctx;
    ctx.path = "/A/B/C/a.c";
    ctx.message = message;
    va_start(ctx.message_args, message);
    mys_string_t *output = mys_fmtex_apply(fmtex, &ctx);
    va_end(ctx.message_args);
    return output;
}

int main() {
    mys_fmter_t *fmter = mys_fmter_create();
    mys_fmter_register_pass(fmter, "asctime", fpass_asctime);
    mys_fmter_register_pass(fmter, "path", fpass_path);
    mys_fmter_register_pass(fmter, "message", fpass_message);
    mys_fmtex_t *fmtex1 = mys_fmter_compile(fmter, "[{asctime:%Y/%m/%d} {path:l}]{color:none} |{message}|\n");
    mys_fmtex_t *fmtex2 = mys_fmter_compile(fmter, "[{path:s}] {message}\n");

    mys_string_t *result1 = trylog(fmtex1, "Hello world with style %d", 1);
    mys_string_t *result2 = trylog(fmtex2, "Goodbye world with style %d", 2);
    printf("%s", result1->text);
    printf("%s", result2->text);
    mys_string_destroy(&result1);
    mys_string_destroy(&result2);

    mys_fmtex_free(&fmtex1);
    mys_fmtex_free(&fmtex2);
    mys_fmter_destroy(&fmter);

    return 0;
}
*/
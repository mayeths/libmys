#ifndef __MYS_IMPL_H__
#define __MYS_IMPL_H__

#include "thread.h"
#include "errno.h"
#include "log.h"

/*********************************************/
// C definition
/*********************************************/
mys_thread_local int mys_errno = 0;
mys_log_G_t mys_log_G = {
    .level = MYS_LOG_TRACE,
    .last_level = MYS_LOG_TRACE,
    .lock = MYS_MUTEX_INITIALIZER,
    .id_generator = 10001, /* DO NOT start from 0 */
    .handlers = {
        { .fn = __mys_stdout_handler, .udata = NULL, .id = 10000 },
        { .fn = NULL, .udata = NULL, .id = 0 /* Uninitalized ID is 0 */ },
    },
};

/*********************************************/
// C++ definition
/*********************************************/
#ifdef __cplusplus
#endif /*__cplusplus*/

#endif /*__MYS_IMPL_H__*/
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
    .handlers = {
#ifndef MYS_LOG_DISABLE_STDOUT_HANDLER
        { .fn = __mys_stdio_handler, .udata = NULL, .id = 10000 },
#endif
        { .fn = NULL, .udata = NULL, .id = 0 /* Uninitalized ID is 0 */ },
    },
};

/*********************************************/
// C++ definition
/*********************************************/
#ifdef __cplusplus
#endif /*__cplusplus*/

#endif /*__MYS_IMPL_H__*/
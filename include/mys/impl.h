#ifndef __MYS_IMPL_H__
#define __MYS_IMPL_H__

#include "thread.h"
#include "errno.h"

/*********************************************/
// C definition
/*********************************************/
mys_thread_local int mys_errno = 0;
mys_mutex_t mys_logging_mutex = MYS_MUTEX_INITIALIZER;

/*********************************************/
// C++ definition
/*********************************************/
#ifdef __cplusplus
#endif /*__cplusplus*/

#endif /*__MYS_IMPL_H__*/
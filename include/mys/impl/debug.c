#include "_private.h"
#include "../debug.h"

#define _MYS_DEBUG_STRIP_DEPTH 2
#define _MYS_DEBUG_SIGNAL_MAX 64 // maximum signal to handle
#define _MYS_DEBUG_BACKTRACE_MAX 64 // maximum stack backtrace
#define _MYS_DEBUG_SBUF_NUM  3 // addr2line bufcmd, addr2line bufout, cause
#define _MYS_DEBUG_SBUF_SIZE 256ULL // for holding small message
#define _MYS_DEBUG_LBUF_SIZE 65536ULL // for holding the entire backtrace
#define _MYS_DEBUG_ADDITIONAL_SIZE (128ULL * 1024) // preserve additional stack space
#define _MYS_DEBUG_STACK_SIZE ( \
    (SIGSTKSZ) + \
    (_MYS_DEBUG_BACKTRACE_MAX * sizeof(void*)) + \
    (_MYS_DEBUG_SBUF_SIZE * _MYS_DEBUG_SBUF_NUM) + \
    (_MYS_DEBUG_LBUF_SIZE) + \
    (_MYS_DEBUG_ADDITIONAL_SIZE) \
)

struct _mys_debug_G_t {
    mys_mutex_t lock;
    bool inited;
    int outfd;
    int use_color;
    int post_action;
    int max_frames;
    char message[MYS_DEBUG_MESSAGE_MAX];
    //
    int nsignals;
    int signals[_MYS_DEBUG_SIGNAL_MAX];
    uint8_t *stack_memory;
    stack_t stack;
    stack_t old_stack;
    struct sigaction old_actions[_MYS_DEBUG_SIGNAL_MAX];
    // timeout
    bool timeout_inited;
    bool timeout_reached;
    int timeout_signal;
    struct sigaction timeout_old_action;
    timer_t timeout_id;
    double timeout_start;
    double timeout;
    pid_t process_pid;
    const char *timeout_set_file;
    int timeout_set_line;
    // filter
    size_t n_filters;
    size_t cap_filters;
    char **filters;
};

static struct _mys_debug_G_t _mys_debug_G = {
    .lock = MYS_MUTEX_INITIALIZER,
    .inited = false,
    .outfd = STDERR_FILENO,
    .use_color = 0,
    .post_action = MYS_DEBUG_ACTION_EXIT,
    .max_frames = _MYS_DEBUG_BACKTRACE_MAX,
    .message = {},
    .nsignals = 0,
    .signals = {},
    .stack_memory = NULL,
    .stack = {},
    .old_stack = {},
    .old_actions = {},
    .timeout_inited = false,
    .timeout_reached = false,
    .timeout_signal = -1,
    .timeout_old_action = {},
    .timeout_id = NULL,
    .timeout_start = 0,
    .timeout = 0,
    .process_pid = -1,
    .timeout_set_file = NULL,
    .timeout_set_line = -1,
    .n_filters = 0,
    .cap_filters = 0,
    .filters = NULL,
};

MYS_STATIC void _mys_debug_signal_handler(int signo, siginfo_t *info, void *context);
MYS_STATIC void _mys_debug_revert_all();
MYS_STATIC int  _mys_debug_set_signal(int signo, bool is_called_by_user);
MYS_STATIC const char *_mys_sigcause(int signo, int sigcode);
MYS_STATIC const char *_mys_signo_str(int signo);

MYS_API void mys_debug_init()
{
    // https://stackoverflow.com/a/61860187/11702338
    // Make sure that backtrace(libgcc) is loaded before any signals are generated
    void* dummy = NULL;
    backtrace(&dummy, 1);

    mys_mutex_lock(&_mys_debug_G.lock);
    if (!_mys_debug_G.inited) {
        _mys_debug_G.use_color = isatty(_mys_debug_G.outfd);
        memset(_mys_debug_G.message, 0, MYS_DEBUG_MESSAGE_MAX);

        _mys_debug_G.nsignals = 0;
        memset(_mys_debug_G.signals, 0, sizeof(int) * _MYS_DEBUG_SIGNAL_MAX);
        /***** Signal that terminate the process *****/
        _mys_debug_set_signal(SIGALRM, 0);      // P1990 | Alarm clock
        // _mys_debug_set_signal(SIGHUP, 0);    // P1990 | Hangup (when terminal is closed)
        // _mys_debug_set_signal(SIGINT, 0);    // P1990 | Terminal interrupt signal (Ctrl-C)
        // _mys_debug_set_signal(SIGKILL, 0);   // P1990 | Kill immediately <!cannot be caught or ignored>
        _mys_debug_set_signal(SIGPIPE, 0);      // P1990 | Write on a pipe with no one to read it
#ifdef SIGPOLL
        _mys_debug_set_signal(SIGPOLL, 0);      // P2001 | Pollable event
#endif
        // _mys_debug_set_signal(SIGPROF, 0);   // P2001 | Profiling timer expired
        _mys_debug_set_signal(SIGTERM, 0);      // P1990 | Termination signal
        // _mys_debug_set_signal(SIGUSR1, 0);   // P1990 | User-defined signal 1
        // _mys_debug_set_signal(SIGUSR2, 0);   // P1990 | User-defined signal 2
        // _mys_debug_set_signal(SIGVTALRM, 0); // P2001 | Virtual timer expired
        /***** Signal that terminate the process with core dump *****/
        _mys_debug_set_signal(SIGABRT, 0);      // P1990 | Process abort signal
        _mys_debug_set_signal(SIGBUS, 0);       // P2001 | Access to an undefined portion of a memory object
        _mys_debug_set_signal(SIGFPE, 0);       // P1990 | Erroneous arithmetic operation
        _mys_debug_set_signal(SIGILL, 0);       // P1990 | Illegal instruction
        // _mys_debug_set_signal(SIGQUIT, 0);   // P1990 | Terminal quit signal (Ctrl-\\)
        _mys_debug_set_signal(SIGSEGV, 0);      // P1990 | Invalid memory reference
        _mys_debug_set_signal(SIGSYS, 0);       // P2001 | Bad system call
        // _mys_debug_set_signal(SIGTRAP, 0);   // P2001 | Trace/breakpoint trap
        _mys_debug_set_signal(SIGXCPU, 0);      // P2001 | CPU time limit exceeded
        _mys_debug_set_signal(SIGXFSZ, 0);      // P2001 | File size limit exceeded
        /***** Signal that is ignored by default *****/
        // _mys_debug_set_signal(SIGCHLD, 0);   // P1990 | Child process terminated, stopped, or continued
        // _mys_debug_set_signal(SIGURG, 0);    // P2001 | Out-of-band data is available at a socket
        /***** Signal that suspend the process for later resumption *****/
        // _mys_debug_set_signal(SIGSTOP, 0);   // P1990 | System stop sinal <!cannot be caught or ignored>
        // _mys_debug_set_signal(SIGTSTP, 0);   // P1990 | Terminal stop signal by user (Ctrl-Z)
        // _mys_debug_set_signal(SIGTTIN, 0);   // P1990 | Background process attempting read
        // _mys_debug_set_signal(SIGTTOU, 0);   // P1990 | Background process attempting write
        /***** Signal that continue the process if it's stopped *****/
        // _mys_debug_set_signal(SIGCONT, 0);   // P1990 | Continue executing

        _mys_debug_G.stack_memory = (uint8_t *)malloc(_MYS_DEBUG_STACK_SIZE);
        memset(_mys_debug_G.stack_memory, 0, _MYS_DEBUG_STACK_SIZE);
#ifndef OS_MACOS // macos lost backtrace if run signal handler on new stack
        stack_t *stack = &_mys_debug_G.stack;
        stack_t *old_stack = &_mys_debug_G.old_stack;
        stack->ss_sp = _mys_debug_G.stack_memory;
        stack->ss_size = _MYS_DEBUG_STACK_SIZE;
        stack->ss_flags = 0;
        if (-1 == sigaltstack(stack, old_stack)) {
            printf("sigaltstack failed: %s\n", strerror(errno));
            exit(1);
        }
#endif

        _mys_debug_G.process_pid = getpid();
        _mys_debug_G.timeout_inited = false;
        _mys_debug_G.timeout_signal = -1;
        _mys_debug_G.n_filters = 0;
        _mys_debug_G.cap_filters = 0;
        _mys_debug_G.filters = NULL;
        _mys_debug_G.inited = true;
    }
    mys_mutex_unlock(&_mys_debug_G.lock);
}

MYS_API void mys_debug_fini()
{
    mys_mutex_lock(&_mys_debug_G.lock);
    if (_mys_debug_G.inited) {
        _mys_debug_revert_all();
        free(_mys_debug_G.stack_memory);
        _mys_debug_G.inited = false;
    }
    mys_mutex_unlock(&_mys_debug_G.lock);
}

MYS_API int mys_debug_set_signal(int signo)
{
    return _mys_debug_set_signal(signo, 1);
}

MYS_API int mys_debug_clear_signal(int signo)
{
    if (_mys_debug_G.inited) {
        if (signo <= 0)
            return 1;
        int index;
        for (index = 0; index < _mys_debug_G.nsignals; index++) {
            if (_mys_debug_G.signals[index] == signo)
                break;
        }
        struct sigaction action;
        if (-1 == sigaction(signo, &_mys_debug_G.old_actions[index], &action)) {
            printf("failed to clear signal handler for signal %d : %s\n",
                _mys_debug_G.signals[index], strerror(errno));
            exit(1);
        }
        for (; index < _mys_debug_G.nsignals - 1; index++) {
            _mys_debug_G.signals[index] = _mys_debug_G.signals[index + 1];
        }
        _mys_debug_G.signals[_mys_debug_G.nsignals - 1] = 0;
        _mys_debug_G.nsignals -= 1;
        return 0;
    }
    return 1;
}

MYS_API void mys_debug_get_message(char *buffer)
{
    mys_mutex_lock(&_mys_debug_G.lock);
    {
        memcpy(buffer, _mys_debug_G.message, MYS_DEBUG_MESSAGE_MAX);
    }
    mys_mutex_unlock(&_mys_debug_G.lock);
}

MYS_API void mys_debug_set_message(const char *fmt, ...)
{
    mys_mutex_lock(&_mys_debug_G.lock);
    {
        va_list args;
        va_start(args, fmt);
        vsnprintf(_mys_debug_G.message, MYS_DEBUG_MESSAGE_MAX, fmt, args);
        va_end(args);
    }
    mys_mutex_unlock(&_mys_debug_G.lock);
}

MYS_API void mys_debug_clear_message()
{
    mys_mutex_lock(&_mys_debug_G.lock);
    {
        _mys_debug_G.message[0] = '\0';
    }
    mys_mutex_unlock(&_mys_debug_G.lock);
}

MYS_API void mys_debug_set_max_frames(int max_frames)
{
    // FIXME: lock
    _mys_debug_G.max_frames = max_frames;
}

MYS_API int mys_debug_get_max_frames()
{
    // FIXME: lock
    return _mys_debug_G.max_frames;
}


////////////
//////////// static functions
////////////


MYS_STATIC int _mys_debug_set_signal(int signo, bool is_called_by_user)
{
    if (is_called_by_user && !_mys_debug_G.inited)
        return 1;
    if (signo <= 0 || _mys_debug_G.nsignals >= _MYS_DEBUG_SIGNAL_MAX)
        return 1;

    for (int i = 0; i < _mys_debug_G.nsignals; i++) {
        if (_mys_debug_G.signals[i] == signo)
            return 0;
    }

    struct sigaction new_action, old_action;
    new_action.sa_sigaction = _mys_debug_signal_handler;
    new_action.sa_flags = SA_SIGINFO | SA_ONSTACK;
    sigemptyset(&new_action.sa_mask);
    if (-1 == sigaction(signo, &new_action, &old_action)) {
        printf("failed to set signal handler for signo %d : %s\n", signo, strerror(errno));
        exit(1);
    }
    _mys_debug_G.signals[_mys_debug_G.nsignals] = signo;
    _mys_debug_G.old_actions[_mys_debug_G.nsignals] = old_action;
    _mys_debug_G.nsignals += 1;
    return 0;
}

MYS_STATIC const char *_mys_sigcause(int signo, int sigcode)
{
    if (signo == SIGILL) {
        switch (sigcode) {
            case ILL_ILLOPC : return "illegal opcode";
            case ILL_ILLOPN : return "illegal operand";
            case ILL_ILLADR : return "illegal addressing mode";
            case ILL_ILLTRP : return "illegal trap";
            case ILL_PRVOPC : return "privileged opcode";
            case ILL_PRVREG : return "privileged register";
            case ILL_COPROC : return "coprocessor error";
            case ILL_BADSTK : return "internal stack error";
        }
    } else if (signo == SIGTRAP) {
        switch (sigcode) {
            case TRAP_BRKPT : return "process breakpoint";
            case TRAP_TRACE : return "process trace trap";
        }
    } else if (signo == SIGBUS) {
        switch (sigcode) {
            case BUS_ADRALN : return "invalid address alignment";
            case BUS_ADRERR : return "nonexistent physical address";
            case BUS_OBJERR : return "object-specific hardware error";
        }
    } else if (signo == SIGFPE) {
        switch (sigcode) {
            case FPE_INTDIV : return "integer divide by zero";
            case FPE_INTOVF : return "integer overflow";
            case FPE_FLTDIV : return "floating-point divide by zero";
            case FPE_FLTOVF : return "floating-point overflow";
            case FPE_FLTUND : return "floating-point underflow";
            case FPE_FLTRES : return "floating-point inexact result";
            case FPE_FLTINV : return "floating-point invalid operation";
            case FPE_FLTSUB : return "subscript bufout of range";
        }
    } else if (signo == SIGSEGV) {
        switch (sigcode) {
            case SEGV_MAPERR : return "address not mapped to object";
            case SEGV_ACCERR : return "invalid permissions for mapped object";
        }
    } else if (signo == SIGCHLD) {
        switch (sigcode) {
            case CLD_EXITED    : return "child has exited";
            case CLD_KILLED    : return "child was killed";
            case CLD_DUMPED    : return "child terminated abnormally";
            case CLD_TRAPPED   : return "traced child has trapped";
            case CLD_STOPPED   : return "child has stopped";
            case CLD_CONTINUED : return "stopped child has continued";
        }
    } else { // common
        switch (sigcode) {
            case SI_USER    : return "sent by kill(2) or raise(3)";
#ifdef __linux__
            case SI_KERNEL  : return "sent by kernel";
#endif
            case SI_QUEUE   : return "sent by sigqueue(2)";
            case SI_TIMER   : return "sent by POSIX timer expiration";
            case SI_MESGQ   : return "sent by POSIX message queue state change";
            case SI_ASYNCIO : return "sent by AIO completion";
#ifdef SI_SIGIO
            case SI_SIGIO   : return "sent by queued SIGIO";
#endif
#ifdef SI_TKILL
            case SI_TKILL   : return "sent by tkill(2) or tgkill(2)";
#endif
        }
    }
    return "<unknown reason>";
}

MYS_STATIC const char *_mys_signo_str(int signo)
{
    switch (signo) {
    case SIGALRM: return "SIGALRM";
    case SIGHUP: return "SIGHUP";
    case SIGINT: return "SIGINT";
    case SIGKILL: return "SIGKILL";
    case SIGPIPE: return "SIGPIPE";
#ifdef SIGPOLL
    case SIGPOLL: return "SIGPOLL";
#endif
    case SIGPROF: return "SIGPROF";
    case SIGTERM: return "SIGTERM";
    case SIGUSR1: return "SIGUSR1";
    case SIGUSR2: return "SIGUSR2";
    case SIGVTALRM: return "SIGVTALRM";
    case SIGABRT: return "SIGABRT";
    case SIGBUS: return "SIGBUS";
    case SIGFPE: return "SIGFPE";
    case SIGILL: return "SIGILL";
    case SIGQUIT: return "SIGQUIT";
    case SIGSEGV: return "SIGSEGV";
    case SIGSYS: return "SIGSYS";
    case SIGTRAP: return "SIGTRAP";
    case SIGXCPU: return "SIGXCPU";
    case SIGXFSZ: return "SIGXFSZ";
    case SIGCHLD: return "SIGCHLD";
    case SIGURG: return "SIGURG";
    case SIGSTOP: return "SIGSTOP";
    case SIGTSTP: return "SIGTSTP";
    case SIGTTIN: return "SIGTTIN";
    case SIGTTOU: return "SIGTTOU";
    case SIGCONT: return "SIGCONT";
    }
    return "SIGUNKNOWN";
}

MYS_STATIC void _mys_debug_revert_all()
{
    mys_mutex_lock(&_mys_debug_G.lock);
    {
        char msg[256];
        for (int i = 0; i < _MYS_DEBUG_SIGNAL_MAX; ++i) {
            int sig = _mys_debug_G.signals[i];
            if (sig == 0)
                break;
            struct sigaction action;
            int ret = sigaction(sig, &_mys_debug_G.old_actions[i], &action);
            if (ret != 0) {
                snprintf(msg, sizeof(msg), "failed to revert signal handler for sig %d : %s\n",
                    sig, strerror(errno));
                write(_mys_debug_G.outfd, msg, strnlen(msg, sizeof(msg)));
            }
        }
    }
    mys_mutex_unlock(&_mys_debug_G.lock);
}

MYS_NOINLINE
MYS_STATIC void _mys_debug_signal_handler(int signo, siginfo_t *info, void *context)
{
    (void)context;
    int code = info->si_code;
    void *addr = info->si_addr;
    _mys_debug_revert_all(); // ask old handlers to clean up in case our handler crash.

    char cause[_MYS_DEBUG_SBUF_SIZE];
    char bufcmd[_MYS_DEBUG_SBUF_SIZE];
    char bufout[_MYS_DEBUG_SBUF_SIZE];
    char buflog[_MYS_DEBUG_LBUF_SIZE];
    void *baddrs[_MYS_DEBUG_BACKTRACE_MAX];
    int myrank = mys_mpi_myrank();
    int nranks = mys_mpi_nranks();
    int digits = mys_math_trunc(mys_math_log10(nranks)) + 1;
    digits = digits > 3 ? digits : 3;
    size_t loglen = 0;
    size_t logmax = sizeof(buflog);
    bool is_timeout = _mys_debug_G.timeout_inited && signo == _mys_debug_G.timeout_signal && _mys_debug_G.timeout_reached;

    switch (signo) {
#define _MYS_CASE_SIG(s, fmt, ...) s:                       \
        snprintf(cause, sizeof(cause), fmt, ##__VA_ARGS__); \
        break;
    _MYS_CASE_SIG(case SIGSEGV, "%s at %p", _mys_sigcause(signo, code), addr);
    _MYS_CASE_SIG(case SIGCHLD, "%s at %p", _mys_sigcause(signo, code), addr);
    _MYS_CASE_SIG(case SIGILL,  "%s", _mys_sigcause(signo, code));
    _MYS_CASE_SIG(case SIGTRAP, "%s", _mys_sigcause(signo, code));
    _MYS_CASE_SIG(case SIGBUS,  "%s", _mys_sigcause(signo, code));
    _MYS_CASE_SIG(case SIGFPE,  "%s", _mys_sigcause(signo, code));
#undef _MYS_CASE_SIG
    default: {
        if (is_timeout)
            snprintf(cause, sizeof(cause), "Reached %f seconds timeout (set at %s:%d)", _mys_debug_G.timeout, _mys_debug_G.timeout_set_file, _mys_debug_G.timeout_set_line);
        else
            snprintf(cause, sizeof(cause), "%s", _mys_sigcause(signo, code));
        break;
    }
    }

#define _DOFMT(f, ...) do {                                                     \
    if (loglen < logmax)                                                        \
        loglen += snprintf(buflog + loglen, logmax - loglen, f, ##__VA_ARGS__); \
} while (0)
#define _NFMT1 "[F::%0*d CRASH] -------------------------------\n"
#define _NFMT2 "[F::%0*d CRASH] | %s (%s)\n"
#define _NFMT3 "[F::%0*d CRASH] | %s\n"
#define _NFMT4 "[F::%0*d CRASH] | No backtrace stack available\n"
#define _NFMT5 "[F::%0*d CRASH] | (Filtered %d frames)\n"
#define _NFMT6 "[F::%0*d CRASH] -------------------------------\n"
#define _YFMT1 MCOLOR_RED "[F::%0*d CRASH] -------------------------------\n"
#define _YFMT2 "[F::%0*d CRASH] |" MCOLOR_BOLD " %s (%s)" MCOLOR_NO MCOLOR_RED "\n"
#define _YFMT3 "[F::%0*d CRASH] |" MCOLOR_BOLD " %s" MCOLOR_NO MCOLOR_RED "\n"
#define _YFMT4 "[F::%0*d CRASH] |" MCOLOR_BOLD " No backtrace stack available" MCOLOR_NO MCOLOR_RED "\n"
// #define _YFMT5 "[F::%0*d CRASH] |" MCOLOR_BOLD " Collapsed %d frames for filter settings" MCOLOR_NO MCOLOR_RED "\n"
#define _YFMT5 "[F::%0*d CRASH] | (Filtered %d frames)\n"
#define _YFMT6 "[F::%0*d CRASH] -------------------------------" MCOLOR_NO "\n"
    {
        const char *self_exe = mys_procname();
        int bsize = backtrace(baddrs, _MYS_DEBUG_STRIP_DEPTH + _mys_debug_G.max_frames);
        int bdigits = mys_math_trunc(mys_math_log10(bsize)) + 1;
        bdigits = bdigits > 2 ? bdigits : 2;
        char **bsyms = backtrace_symbols(baddrs, bsize);

        int color = _mys_debug_G.use_color;

        _DOFMT(color ? _YFMT1 : _NFMT1, digits, myrank);
        if (is_timeout) {
            _DOFMT(color ? _YFMT3 : _NFMT3, digits, myrank, cause);
        } else {
            _DOFMT(color ? _YFMT2 : _NFMT2, digits, myrank, strsignal(signo), cause);
        }
        if (_mys_debug_G.message[0] != '\0')
            _DOFMT(color ? _YFMT3 : _NFMT3, digits, myrank, _mys_debug_G.message);
        if (bsize == 0)
            _DOFMT(color ? _YFMT4 : _NFMT4, digits, myrank);

        int collapsed = 0;
        for (int i = _MYS_DEBUG_STRIP_DEPTH; i < bsize; ++i) {
            snprintf(bufcmd, sizeof(bufcmd), "addr2line -e %s %p", self_exe, baddrs[i]);
            mys_prun_t run = mys_prun_create(bufcmd, bufout, sizeof(bufout), NULL, 0);
            bool skip = false;
            for (size_t j = 0; j < _mys_debug_G.n_filters; j++) {
                if (strstr(bsyms[i], _mys_debug_G.filters[j])) {
                    skip = true;
                    break;
                }
                if (strstr(bufout, _mys_debug_G.filters[j])) {
                    skip = true;
                    break;
                }
            }
            if (skip) {
                collapsed += 1;
                continue;
            }
            _DOFMT("[F::%0*d CRASH] | %-*d %s at %s\n",
                digits, myrank, bdigits, i - _MYS_DEBUG_STRIP_DEPTH, bsyms[i], bufout);
            mys_prun_destroy(&run);
        }
        if (collapsed != 0)
            _DOFMT(color ? _YFMT5 : _NFMT5, digits, myrank, collapsed);
        free(bsyms);
        _DOFMT(color ? _YFMT6 : _NFMT6, digits, myrank);
    }

#undef _DOFMT
#undef _NFMT1
#undef _NFMT2
#undef _NFMT3
#undef _NFMT4
#undef _NFMT5
#undef _NFMT6
#undef _YFMT1
#undef _YFMT2
#undef _YFMT3
#undef _YFMT4
#undef _YFMT5
#undef _YFMT6
    write(_mys_debug_G.outfd, buflog, loglen);
    int post_action = _mys_debug_G.post_action;
    if (post_action == MYS_DEBUG_ACTION_EXIT) {
        // when there are a lot of ranks, immediately exit will
        // cause some of them failed to output information.
        sleep(1);
        _exit(signo);
    } else if (post_action == MYS_DEBUG_ACTION_RAISE) {
        raise(signo);
    } else if (post_action == MYS_DEBUG_ACTION_FREEZE) {
        do {} while (1);
    }
}



#ifdef MYS_ENABLE_DEBUG_TIMEOUT
static void _mys_debug_timeout_handler(union sigval sv)
{
    (void)sv;
    timer_delete(_mys_debug_G.timeout_id);
    _mys_debug_G.timeout_reached = true;
    // FIXME: Seems like sometime we receive the signal instead of parent thread?
    // Should we clear signal handler first?
    if (kill(_mys_debug_G.process_pid, _mys_debug_G.timeout_signal) == -1) {
        perror("_mys_debug_timeout_handler timeout kill failed");
    }
}


MYS_API void _mys_debug_set_timeout(double timeout, const char *file, int line)
{
    if (!_mys_debug_G.inited) {
        printf("call mys_debug_init() before mys_debug_set_timeout()\n");
        exit(1);
    }
    if (!_mys_debug_G.timeout_inited) {
        struct sigaction new_action;
        new_action.sa_sigaction = _mys_debug_signal_handler;
        new_action.sa_flags = SA_SIGINFO | SA_ONSTACK;
        sigemptyset(&new_action.sa_mask);
#ifdef OS_MACOS
        _mys_debug_G.timeout_signal = SIGUSR2;
#else
        _mys_debug_G.timeout_signal = SIGRTMIN + 1;
#endif
        if (-1 == sigaction(_mys_debug_G.timeout_signal, &new_action, &_mys_debug_G.timeout_old_action)) {
            printf("failed to set signal handler for timeout signo %d : %s\n", _mys_debug_G.timeout_signal, strerror(errno));
            exit(1);
        }
        struct sigevent sev;
        memset(&sev, 0, sizeof(struct sigevent));
        sev.sigev_notify = SIGEV_THREAD;
        sev.sigev_notify_function = &_mys_debug_timeout_handler;
        sev.sigev_notify_attributes = NULL;
        if (timer_create(CLOCK_REALTIME, &sev, &_mys_debug_G.timeout_id) == -1) {
            printf("failed to create timeout timer: %s\n", strerror(errno));
            exit(1);
        }
        _mys_debug_G.timeout_reached = false;
        _mys_debug_G.timeout_set_file = file;
        _mys_debug_G.timeout_set_line = line;
        _mys_debug_G.timeout_inited = true;
    }
    struct itimerspec its;
    its.it_value.tv_sec = (long)timeout;
    its.it_value.tv_nsec = (long)((timeout - (double)its.it_value.tv_sec) * 1e9);
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;
    if (timer_settime(_mys_debug_G.timeout_id, 0, &its, NULL) == -1) {
        printf("failed to set timeout timer: %s\n", strerror(errno));
        exit(1);
    }
    _mys_debug_G.timeout = timeout;
}

MYS_API void mys_debug_clear_timeout()
{
    if (!_mys_debug_G.timeout_inited) {
        return;
    }
    struct itimerspec its;
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;
    if (timer_settime(_mys_debug_G.timeout_id, 0, &its, NULL) == -1) {
        printf("failed to set timeout timer: %s\n", strerror(errno));
        exit(1);
    }
    //
    if (-1 == sigaction(_mys_debug_G.timeout_signal, &_mys_debug_G.timeout_old_action, NULL)) {
        printf("failed to set previous signal handler for timeout signo %d : %s\n", _mys_debug_G.timeout_signal, strerror(errno));
        exit(1);
    }
    if (-1 == timer_delete(_mys_debug_G.timeout_id)) {
        printf("failed to delete timeout timer: %s\n", strerror(errno));
        exit(1);
    }
    _mys_debug_G.timeout_reached = false;
    _mys_debug_G.timeout_inited = false;
}
#endif /*MYS_ENABLE_DEBUG_TIMEOUT*/

MYS_API void mys_debug_add_stack_filter(const char *match_str)
{
    if (_mys_debug_G.n_filters == _mys_debug_G.cap_filters) {
        if (_mys_debug_G.cap_filters == 0)
            _mys_debug_G.cap_filters = 1;
        else
            _mys_debug_G.cap_filters *= 2;
        _mys_debug_G.filters = (char **)realloc(_mys_debug_G.filters, sizeof(char *) * _mys_debug_G.cap_filters);
    }
    _mys_debug_G.filters[_mys_debug_G.n_filters++] = strdup(match_str);
}

MYS_API void mys_debug_del_stack_filter(const char *match_str)
{
    size_t j = 0;
    bool found = false;
    for (j = 0; j < _mys_debug_G.n_filters; j++) {
        if (0 == strcmp(match_str, _mys_debug_G.filters[j])) {
            found = true;
            break;
        }
    }
    if (found) {
        free(_mys_debug_G.filters[j]);
        for (size_t i = j + 1; i < _mys_debug_G.n_filters; i++) {
            _mys_debug_G.filters[i - 1] = _mys_debug_G.filters[i];
        }
        _mys_debug_G.n_filters -= 1;
    }
}


#ifdef OS_MACOS

MYS_STATIC void _timer_cancel(void *arg)
{
    struct macos_timer *tim = (struct macos_timer *)arg;
    dispatch_release(tim->tim_timer);
    dispatch_release(tim->tim_queue);
    tim->tim_timer = NULL;
    tim->tim_queue = NULL;
    free(tim);
}

MYS_STATIC void _timer_handler(void *arg)
{
    struct macos_timer *tim = (struct macos_timer *)arg;
    union sigval sv;

    sv.sival_ptr = tim->tim_arg;

    if (tim->tim_func != NULL)
        tim->tim_func(sv);
}

MYS_STATIC int timer_create(clockid_t clockid, struct sigevent *sevp, timer_t *timerid)
{
    struct macos_timer *tim;

    *timerid = NULL;

    switch (clockid) {
        case CLOCK_REALTIME:

            /* What is implemented so far */
            if (sevp->sigev_notify != SIGEV_THREAD) {
                errno = ENOTSUP;
                return (-1);
            }

            tim = (struct macos_timer *)malloc(sizeof(struct macos_timer));
            if (tim == NULL) {
                errno = ENOMEM;
                return (-1);
            }

            tim->tim_queue = dispatch_queue_create("com.mayeths.timerqueue", 0);
            tim->tim_timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, tim->tim_queue);

            tim->tim_func = sevp->sigev_notify_function;
            tim->tim_arg  = sevp->sigev_value.sival_ptr;
            *timerid      = tim;

            /* Opting to use pure C instead of Block versions */
            dispatch_set_context(tim->tim_timer, tim);
            dispatch_source_set_event_handler_f(tim->tim_timer, _timer_handler);
            dispatch_source_set_cancel_handler_f(tim->tim_timer, _timer_cancel);

            return (0);
        default:
            break;
    }

    errno = EINVAL;
    return (-1);
}

MYS_STATIC int timer_settime(timer_t tim, int flags, const struct itimerspec *its, struct itimerspec *remainvalue)
{
    (void)flags;
    (void)remainvalue;
    if (tim != NULL) {
        /* Both zero, is disarm */
        if (its->it_value.tv_sec == 0 && its->it_value.tv_nsec == 0) {
            /* There's a comment about suspend count in Apple docs */
            dispatch_suspend(tim->tim_timer);
            return (0);
        }

        dispatch_time_t start;
        start = dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC * its->it_value.tv_sec + its->it_value.tv_nsec);
        dispatch_source_set_timer(
            tim->tim_timer, start, NSEC_PER_SEC * its->it_value.tv_sec + its->it_value.tv_nsec, 0
        );
        dispatch_resume(tim->tim_timer);
    }
    return (0);
}

MYS_STATIC int timer_delete(timer_t tim)
{
    /* Calls _timer_cancel() */
    if (tim != NULL)
        dispatch_source_cancel(tim->tim_timer);

    return (0);
}

#endif // OS_MACOS

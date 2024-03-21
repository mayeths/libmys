#include "../checkpoint.h"
#include "uthash_hash.h"

// extern char *strndup(const char *s, size_t n) __THROW;

// https://troydhanson.github.io/uthash/userguide.html#_string_keys
typedef struct _mys_chk_name_t {
    char *name;
    _mys_UT_hash_handle hh;
} _mys_chk_name_t;

static _mys_chk_name_t *checkpoint_name_insert(_mys_chk_name_t **head, const char *name)
{
    _mys_chk_name_t *s = (_mys_chk_name_t *)malloc(sizeof(_mys_chk_name_t));
    s->name = strndup(name, 4096);
    _HASH_ADD_KEYPTR(hh, *head, s->name, strlen(s->name), s);
    return s;
}

static void checkpoint_name_clear(_mys_chk_name_t **head) {
    _mys_chk_name_t *s = NULL;
    _mys_chk_name_t *tmp = NULL;
    _HASH_ITER(hh, *head, s, tmp) {
        _HASH_DEL(*head, s);
        free(s->name);
        free(s);
    }
}

static _mys_chk_name_t *checkpoint_name_find(_mys_chk_name_t *head, const char *name)
{
    _mys_chk_name_t *s = NULL;
    _HASH_FIND_STR(head, name, s);
    return s;
}

typedef struct _mys_chk_t {
    char *name;
    double time;
} _mys_chk_t;

typedef struct _mys_chk_G_t {
    bool inited;
    mys_mutex_t lock;
    double offset;
    size_t size;
    size_t capacity;
    _mys_chk_t *arr;
    _mys_chk_name_t *nameset;
} _mys_chk_G_t;

static _mys_chk_G_t _mys_chk_G = {
    .inited = false,
    .lock = MYS_MUTEX_INITIALIZER,
    .offset = 0,
    .size = 0,
    .capacity = 0,
    .arr = NULL,
    .nameset = NULL,
};

MYS_API void mys_checkpoint_init()
{
    if (_mys_chk_G.inited == true)
        return;
    mys_mutex_lock(&_mys_chk_G.lock);
    if (_mys_chk_G.inited == true)
        return;
    _mys_chk_G.offset = mys_hrtime();
    _mys_chk_G.size = 0;
    _mys_chk_G.capacity = 0;
    _mys_chk_G.arr = NULL;
    _mys_chk_G.nameset = NULL;
    _mys_chk_G.inited = true;
    mys_mutex_unlock(&_mys_chk_G.lock);
}


MYS_API void mys_checkpoint_reset()
{
    mys_checkpoint_init();
    mys_mutex_lock(&_mys_chk_G.lock);
    _mys_chk_G.offset = mys_hrtime();
    _mys_chk_G.size = 0;
    _mys_chk_G.capacity = 0;
    if (_mys_chk_G.arr != NULL) free(_mys_chk_G.arr);
    if (_mys_chk_G.nameset != NULL) checkpoint_name_clear(&_mys_chk_G.nameset);
    _mys_chk_G.arr = NULL;
    _mys_chk_G.nameset = NULL;
    mys_mutex_unlock(&_mys_chk_G.lock);
}

MYS_API void mys_checkpoint(const char *name_format, ...)
{
    mys_checkpoint_init();
    char name[4096];
    va_list args;
    va_start(args, name_format);
    vsnprintf(name, sizeof(name), name_format, args);
    va_end(args);

    mys_mutex_lock(&_mys_chk_G.lock);

    _mys_chk_name_t *child = checkpoint_name_find(_mys_chk_G.nameset, name);
    if (child == NULL)
        child = checkpoint_name_insert(&_mys_chk_G.nameset, name);

    if (_mys_chk_G.size == _mys_chk_G.capacity) {
        _mys_chk_G.capacity = (_mys_chk_G.capacity == 0) ? 128 : _mys_chk_G.capacity * 2;
        size_t bytes = sizeof(_mys_chk_t) * _mys_chk_G.capacity;
        _mys_chk_G.arr = (_mys_chk_t *)realloc(_mys_chk_G.arr, bytes);
    }

    double current = mys_hrtime() - _mys_chk_G.offset;
    _mys_chk_G.arr[_mys_chk_G.size].time = current;
    _mys_chk_G.arr[_mys_chk_G.size].name = child->name;
    _mys_chk_G.size += 1;
    mys_mutex_unlock(&_mys_chk_G.lock);
}

MYS_API int mys_checkpoint_dump(const char *file_format, ...)
{
    mys_checkpoint_init();
    char file[4096];
    va_list args;
    va_start(args, file_format);
    vsnprintf(file, sizeof(file), file_format, args);
    va_end(args);

    mys_mutex_lock(&_mys_chk_G.lock);
    mys_ensure_parent(file, 0777);
    FILE *fd = fopen(file, "w");
    if (fd == NULL)
        return 1;
    fprintf(fd, "name,time\n");
    for (size_t i = 0; i < _mys_chk_G.size; i++) {
        const char *checkpoint_name = _mys_chk_G.arr[i].name;
        double time = _mys_chk_G.arr[i].time;
        fprintf(fd, "%s,%.17e\n", checkpoint_name, time);
    }
    fclose(fd);
    ILOG(0, "Checkpoints Wrote to %s", file);
    mys_mutex_unlock(&_mys_chk_G.lock);

    return 0;
}

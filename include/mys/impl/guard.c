#include "../guard.h"
#include "uthash_list.h"

// https://troydhanson.github.io/uthash/utlist.html
typedef struct _mys_guard_record_t {
    struct _mys_guard_record_t *prev; /* needed for a doubly-linked list only */
    struct _mys_guard_record_t *next; /* needed for singly- or doubly-linked lists */
    uint8_t data[0];
} _mys_guard_record_t;

MYS_STATIC _mys_guard_record_t *_mys_guard_record_create(void *data, size_t size)
{
    _mys_guard_record_t *record = (_mys_guard_record_t *)malloc(sizeof(_mys_guard_record_t) + size);
    record->prev = NULL;
    record->next = NULL;
    memcpy(record->data, data, size);
    return record;
}

MYS_STATIC void _mys_guard_record_destroy(_mys_guard_record_t *record)
{
    free(record);
}

MYS_STATIC _mys_guard_record_t *_mys_guard_records_append(_mys_guard_record_t **head, void *data, size_t size)
{
    _mys_guard_record_t *record = (_mys_guard_record_t *)malloc(sizeof(_mys_guard_record_t) + size);
    record->prev = NULL;
    record->next = NULL;
    memcpy(record->data, data, size);
    _DL_APPEND(*head, record);
    return record;
}

MYS_STATIC int _mys_guard_records_remove(_mys_guard_record_t **head, void *data, size_t size)
{
    _mys_guard_record_t *elt, *tmp;
    int rc = 1;
    _DL_FOREACH_SAFE(*head, elt, tmp) {
        if (memcmp(data, elt->data, size) == 0) {
            _DL_DELETE(*head, elt);
            _mys_guard_record_destroy(elt);
            rc = 0;
        }
    }
    return rc;
}

MYS_STATIC void _mys_guard_records_clear(_mys_guard_record_t **head)
{
    _mys_guard_record_t *elt, *tmp;
    /* now delete each element, use the safe iterator */
    _DL_FOREACH_SAFE(*head, elt, tmp) {
        _DL_DELETE(*head, elt);
        _mys_guard_record_destroy(elt);
    }
}

// https://troydhanson.github.io/uthash/userguide.html#_string_keys
typedef struct _mys_guard_map_t {
    char *type_name;
    size_t type_size;
    size_t num_record;
    _mys_guard_record_t *records;
    _mys_UT_hash_handle hh;
} _mys_guard_map_t;

MYS_STATIC _mys_guard_map_t *_mys_guard_map_find(_mys_guard_map_t *node, const char *name)
{
    _mys_guard_map_t *s = NULL;
    _HASH_FIND_STR(node, name, s);
    return s;
}

MYS_STATIC _mys_guard_map_t *_mys_guard_map_insert(_mys_guard_map_t **node, const char *name, size_t size)
{
    _mys_guard_map_t *s = (_mys_guard_map_t *)malloc(sizeof(_mys_guard_map_t));
    size_t len = strnlen(name, 1024);
    s->type_name = strndup(name, len);
    s->type_size = size;
    s->num_record = 0;
    s->records = NULL;
    _HASH_ADD_KEYPTR(hh, *node, s->type_name, len, s);
    return s;
}

MYS_STATIC void _mys_guard_map_remove(_mys_guard_map_t **head, _mys_guard_map_t *node)
{
    _HASH_DEL(*head, node);
    free(node->type_name);
    _mys_guard_records_clear(&node->records);
    node->num_record = 0;
    free(node);
}

struct _mys_guard_G_t {
    _mys_guard_map_t *map;
};

static struct _mys_guard_G_t _mys_guard_G = {
    .map = NULL,
};

MYS_API void mys_guard_begin(const char *type_name, size_t type_size, void *variable_ptr, const char *file, int line)
{
    if (type_name == NULL) {
        mys_log(mys_mpi_myrank(), MYS_LOG_FATAL, file, line, "(INTERNAL ERROR) Invalid type name (nil).");
        exit(1);
    }
    _mys_guard_map_t *type_node = _mys_guard_map_find(_mys_guard_G.map, type_name);
    if (type_node == NULL) {
        type_node = _mys_guard_map_insert(&_mys_guard_G.map, type_name, type_size);
    }
    _mys_guard_record_t *record = _mys_guard_records_append(&type_node->records, variable_ptr, type_size);
    if (record == NULL) {
        mys_log(mys_mpi_myrank(), MYS_LOG_FATAL, file, line, "(INTERNAL ERROR) Cannot acquire type guard (%s).", type_name);
        exit(1);
    }
    type_node->num_record += 1;
}

MYS_API void mys_guard_end(const char *type_name, size_t type_size, void *variable_ptr, const char *file, int line)
{
    if (type_name == NULL) {
        mys_log(mys_mpi_myrank(), MYS_LOG_FATAL, file, line, "(INTERNAL ERROR) Invalid type name (nil).");
        exit(1);
    }
    _mys_guard_map_t *type_node = _mys_guard_map_find(_mys_guard_G.map, type_name);
    if (type_node == NULL) {
        mys_log(mys_mpi_myrank(), MYS_LOG_FATAL, file, line, "(INTERNAL ERROR) Releasing type guard (%s) that doesn't exist.", type_name);
        exit(1);
    }
    if (_mys_guard_records_remove(&type_node->records, variable_ptr, type_size)) {
        mys_log(mys_mpi_myrank(), MYS_LOG_FATAL, file, line, "Releasing type guard (%s) that didn't begin.", type_name);
        exit(1);
    }
    type_node->num_record -= 1;
    if (type_node->num_record == 0) {
        _mys_guard_map_remove(&_mys_guard_G.map, type_node);
    }
}

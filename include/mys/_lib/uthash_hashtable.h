// https://troydhanson.github.io/uthash/
// Use #define _UTHASH_UNDEF to undef all these things

/************************************************/
/************************************************/
// [Part 1] Define UTHASH
/************************************************/
/************************************************/
#ifdef _UTHASH_DEFINE
#undef _UTHASH_DEFINE
#ifndef _UTHASH_H
#define _UTHASH_H

#define _UTHASH_VERSION 2.3.0

#include <string.h>   /* memcmp, memset, strlen */
#include <stddef.h>   /* ptrdiff_t */
#include <stdlib.h>   /* exit */
#include <stdint.h>   /* uint8_t, uint32_t */

/* These macros use decltype or the earlier __typeof GNU extension.
   As decltype is only available in newer compilers (VS2010 or gcc 4.3+
   when compiling c++ source) this code uses whatever method is needed
   or, for VS2008 where neither is available, uses casting workarounds. */
#if !defined(_DECLTYPE) && !defined(_NO_DECLTYPE)
#if defined(_MSC_VER)   /* MS compiler */
#if _MSC_VER >= 1600 && defined(__cplusplus)  /* VS2010 or newer in C++ mode */
#define _DECLTYPE(x) (decltype(x))
#else                   /* VS2008 or older (or VS2010 in C mode) */
#define _NO_DECLTYPE
#endif
#elif defined(__MCST__)  /* Elbrus C Compiler */
#define _DECLTYPE(x) (__typeof(x))
#elif defined(__BORLANDC__) || defined(__ICCARM__) || defined(__LCC__) || defined(__WATCOMC__)
#define _NO_DECLTYPE
#else                   /* GNU, Sun and other compilers */
#define _DECLTYPE(x) (__typeof(x))
#endif
#endif

#ifdef _NO_DECLTYPE
#define _DECLTYPE(x)
#define _DECLTYPE_ASSIGN(dst,src)                                                 \
do {                                                                             \
  char **_da_dst = (char**)(&(dst));                                             \
  *_da_dst = (char*)(src);                                                       \
} while (0)
#else
#define _DECLTYPE_ASSIGN(dst,src)                                                 \
do {                                                                             \
  (dst) = _DECLTYPE(dst)(src);                                                    \
} while (0)
#endif

#ifndef _uthash_malloc
#define _uthash_malloc(sz) malloc(sz)      /* malloc fcn                      */
#endif
#ifndef _uthash_free
#define _uthash_free(ptr,sz) free(ptr)     /* free fcn                        */
#endif
#ifndef _uthash_bzero
#define _uthash_bzero(a,n) memset(a,'\0',n)
#endif
#ifndef _uthash_strlen
#define _uthash_strlen(s) strlen(s)
#endif

#ifndef _HASH_FUNCTION
#define _HASH_FUNCTION(keyptr,keylen,hashv) _HASH_JEN(keyptr, keylen, hashv)
#endif

#ifndef _HASH_KEYCMP
#define _HASH_KEYCMP(a,b,n) memcmp(a,b,n)
#endif

#ifndef _uthash_noexpand_fyi
#define _uthash_noexpand_fyi(tbl)          /* can be defined to log noexpand  */
#endif
#ifndef _uthash_expand_fyi
#define _uthash_expand_fyi(tbl)            /* can be defined to log expands   */
#endif

#ifndef _HASH_NONFATAL_OOM
#define _HASH_NONFATAL_OOM 0
#endif

#if _HASH_NONFATAL_OOM
/* malloc failures can be recovered from */

#ifndef _uthash_nonfatal_oom
#define _uthash_nonfatal_oom(obj) do {} while (0)    /* non-fatal OOM error */
#endif

#define _HASH_RECORD_OOM(oomed) do { (oomed) = 1; } while (0)
#define _IF_HASH_NONFATAL_OOM(x) x

#else
/* malloc failures result in lost memory, hash tables are unusable */

#ifndef _uthash_fatal
#define _uthash_fatal(msg) exit(-1)        /* fatal OOM error */
#endif

#define _HASH_RECORD_OOM(oomed) _uthash_fatal("out of memory")
#define _IF_HASH_NONFATAL_OOM(x)

#endif

/* initial number of buckets */
#define _HASH_INITIAL_NUM_BUCKETS 32U     /* initial number of buckets        */
#define _HASH_INITIAL_NUM_BUCKETS_LOG2 5U /* lg2 of initial number of buckets */
#define _HASH_BKT_CAPACITY_THRESH 10U     /* expand when bucket count reaches */

/* calculate the element whose hash handle address is hhp */
#define _ELMT_FROM_HH(tbl,hhp) ((void*)(((char*)(hhp)) - ((tbl)->hho)))
/* calculate the hash handle from element address elp */
#define _HH_FROM_ELMT(tbl,elp) ((_mys_UT_hash_handle*)(void*)(((char*)(elp)) + ((tbl)->hho)))

#define _HASH_ROLLBACK_BKT(hh, head, itemptrhh)                                   \
do {                                                                             \
  struct _mys_UT_hash_handle *_hd_hh_item = (itemptrhh);                              \
  unsigned _hd_bkt;                                                              \
  _HASH_TO_BKT(_hd_hh_item->hashv, (head)->hh.tbl->num_buckets, _hd_bkt);         \
  (head)->hh.tbl->buckets[_hd_bkt].count++;                                      \
  _hd_hh_item->hh_next = NULL;                                                   \
  _hd_hh_item->hh_prev = NULL;                                                   \
} while (0)

#define _HASH_VALUE(keyptr,keylen,hashv)                                          \
do {                                                                             \
  _HASH_FUNCTION(keyptr, keylen, hashv);                                          \
} while (0)

#define _HASH_FIND_BYHASHVALUE(hh,head,keyptr,keylen,hashval,out)                 \
do {                                                                             \
  (out) = NULL;                                                                  \
  if (head) {                                                                    \
    unsigned _hf_bkt;                                                            \
    _HASH_TO_BKT(hashval, (head)->hh.tbl->num_buckets, _hf_bkt);                  \
    if (_HASH_BLOOM_TEST((head)->hh.tbl, hashval) != 0) {                         \
      _HASH_FIND_IN_BKT((head)->hh.tbl, hh, (head)->hh.tbl->buckets[ _hf_bkt ], keyptr, keylen, hashval, out); \
    }                                                                            \
  }                                                                              \
} while (0)

#define _HASH_FIND(hh,head,keyptr,keylen,out)                                     \
do {                                                                             \
  (out) = NULL;                                                                  \
  if (head) {                                                                    \
    unsigned _hf_hashv;                                                          \
    _HASH_VALUE(keyptr, keylen, _hf_hashv);                                       \
    _HASH_FIND_BYHASHVALUE(hh, head, keyptr, keylen, _hf_hashv, out);             \
  }                                                                              \
} while (0)

#ifdef _HASH_BLOOM
#define _HASH_BLOOM_BITLEN (1UL << _HASH_BLOOM)
#define _HASH_BLOOM_BYTELEN (_HASH_BLOOM_BITLEN/8UL) + (((_HASH_BLOOM_BITLEN%8UL)!=0UL) ? 1UL : 0UL)
#define _HASH_BLOOM_MAKE(tbl,oomed)                                               \
do {                                                                             \
  (tbl)->bloom_nbits = _HASH_BLOOM;                                               \
  (tbl)->bloom_bv = (uint8_t*)_uthash_malloc(_HASH_BLOOM_BYTELEN);                 \
  if (!(tbl)->bloom_bv) {                                                        \
    _HASH_RECORD_OOM(oomed);                                                      \
  } else {                                                                       \
    _uthash_bzero((tbl)->bloom_bv, _HASH_BLOOM_BYTELEN);                           \
    (tbl)->bloom_sig = _HASH_BLOOM_SIGNATURE;                                     \
  }                                                                              \
} while (0)

#define _HASH_BLOOM_FREE(tbl)                                                     \
do {                                                                             \
  _uthash_free((tbl)->bloom_bv, _HASH_BLOOM_BYTELEN);                              \
} while (0)

#define _HASH_BLOOM_BITSET(bv,idx) (bv[(idx)/8U] |= (1U << ((idx)%8U)))
#define _HASH_BLOOM_BITTEST(bv,idx) (bv[(idx)/8U] & (1U << ((idx)%8U)))

#define _HASH_BLOOM_ADD(tbl,hashv)                                                \
  _HASH_BLOOM_BITSET((tbl)->bloom_bv, ((hashv) & (uint32_t)((1UL << (tbl)->bloom_nbits) - 1U)))

#define _HASH_BLOOM_TEST(tbl,hashv)                                               \
  _HASH_BLOOM_BITTEST((tbl)->bloom_bv, ((hashv) & (uint32_t)((1UL << (tbl)->bloom_nbits) - 1U)))

#else
#define _HASH_BLOOM_MAKE(tbl,oomed)
#define _HASH_BLOOM_FREE(tbl)
#define _HASH_BLOOM_ADD(tbl,hashv)
#define _HASH_BLOOM_TEST(tbl,hashv) (1)
#define _HASH_BLOOM_BYTELEN 0U
#endif

#define _HASH_MAKE_TABLE(hh,head,oomed)                                           \
do {                                                                             \
  (head)->hh.tbl = (_mys_UT_hash_table*)_uthash_malloc(sizeof(_mys_UT_hash_table));         \
  if (!(head)->hh.tbl) {                                                         \
    _HASH_RECORD_OOM(oomed);                                                      \
  } else {                                                                       \
    _uthash_bzero((head)->hh.tbl, sizeof(_mys_UT_hash_table));                         \
    (head)->hh.tbl->tail = &((head)->hh);                                        \
    (head)->hh.tbl->num_buckets = _HASH_INITIAL_NUM_BUCKETS;                      \
    (head)->hh.tbl->log2_num_buckets = _HASH_INITIAL_NUM_BUCKETS_LOG2;            \
    (head)->hh.tbl->hho = (char*)(&(head)->hh) - (char*)(head);                  \
    (head)->hh.tbl->buckets = (_mys_UT_hash_bucket*)_uthash_malloc(                    \
        _HASH_INITIAL_NUM_BUCKETS * sizeof(struct _mys_UT_hash_bucket));               \
    (head)->hh.tbl->signature = _HASH_SIGNATURE;                                  \
    if (!(head)->hh.tbl->buckets) {                                              \
      _HASH_RECORD_OOM(oomed);                                                    \
      _uthash_free((head)->hh.tbl, sizeof(_mys_UT_hash_table));                        \
    } else {                                                                     \
      _uthash_bzero((head)->hh.tbl->buckets,                                      \
          _HASH_INITIAL_NUM_BUCKETS * sizeof(struct _mys_UT_hash_bucket));             \
      _HASH_BLOOM_MAKE((head)->hh.tbl, oomed);                                    \
      _IF_HASH_NONFATAL_OOM(                                                      \
        if (oomed) {                                                             \
          _uthash_free((head)->hh.tbl->buckets,                                   \
              _HASH_INITIAL_NUM_BUCKETS*sizeof(struct _mys_UT_hash_bucket));           \
          _uthash_free((head)->hh.tbl, sizeof(_mys_UT_hash_table));                    \
        }                                                                        \
      )                                                                          \
    }                                                                            \
  }                                                                              \
} while (0)

#define _HASH_REPLACE_BYHASHVALUE_INORDER(hh,head,fieldname,keylen_in,hashval,add,replaced,cmpfcn) \
do {                                                                             \
  (replaced) = NULL;                                                             \
  _HASH_FIND_BYHASHVALUE(hh, head, &((add)->fieldname), keylen_in, hashval, replaced); \
  if (replaced) {                                                                \
    _HASH_DELETE(hh, head, replaced);                                             \
  }                                                                              \
  _HASH_ADD_KEYPTR_BYHASHVALUE_INORDER(hh, head, &((add)->fieldname), keylen_in, hashval, add, cmpfcn); \
} while (0)

#define _HASH_REPLACE_BYHASHVALUE(hh,head,fieldname,keylen_in,hashval,add,replaced) \
do {                                                                             \
  (replaced) = NULL;                                                             \
  _HASH_FIND_BYHASHVALUE(hh, head, &((add)->fieldname), keylen_in, hashval, replaced); \
  if (replaced) {                                                                \
    _HASH_DELETE(hh, head, replaced);                                             \
  }                                                                              \
  _HASH_ADD_KEYPTR_BYHASHVALUE(hh, head, &((add)->fieldname), keylen_in, hashval, add); \
} while (0)

#define _HASH_REPLACE(hh,head,fieldname,keylen_in,add,replaced)                   \
do {                                                                             \
  unsigned _hr_hashv;                                                            \
  _HASH_VALUE(&((add)->fieldname), keylen_in, _hr_hashv);                         \
  _HASH_REPLACE_BYHASHVALUE(hh, head, fieldname, keylen_in, _hr_hashv, add, replaced); \
} while (0)

#define _HASH_REPLACE_INORDER(hh,head,fieldname,keylen_in,add,replaced,cmpfcn)    \
do {                                                                             \
  unsigned _hr_hashv;                                                            \
  _HASH_VALUE(&((add)->fieldname), keylen_in, _hr_hashv);                         \
  _HASH_REPLACE_BYHASHVALUE_INORDER(hh, head, fieldname, keylen_in, _hr_hashv, add, replaced, cmpfcn); \
} while (0)

#define _HASH_APPEND_LIST(hh, head, add)                                          \
do {                                                                             \
  (add)->hh.next = NULL;                                                         \
  (add)->hh.prev = _ELMT_FROM_HH((head)->hh.tbl, (head)->hh.tbl->tail);           \
  (head)->hh.tbl->tail->next = (add);                                            \
  (head)->hh.tbl->tail = &((add)->hh);                                           \
} while (0)

#define _HASH_AKBI_INNER_LOOP(hh,head,add,cmpfcn)                                 \
do {                                                                             \
  do {                                                                           \
    if (cmpfcn(_DECLTYPE(head)(_hs_iter), add) > 0) {                             \
      break;                                                                     \
    }                                                                            \
  } while ((_hs_iter = _HH_FROM_ELMT((head)->hh.tbl, _hs_iter)->next));           \
} while (0)

#ifdef _NO_DECLTYPE
#undef _HASH_AKBI_INNER_LOOP
#define _HASH_AKBI_INNER_LOOP(hh,head,add,cmpfcn)                                 \
do {                                                                             \
  char *_hs_saved_head = (char*)(head);                                          \
  do {                                                                           \
    _DECLTYPE_ASSIGN(head, _hs_iter);                                             \
    if (cmpfcn(head, add) > 0) {                                                 \
      _DECLTYPE_ASSIGN(head, _hs_saved_head);                                     \
      break;                                                                     \
    }                                                                            \
    _DECLTYPE_ASSIGN(head, _hs_saved_head);                                       \
  } while ((_hs_iter = _HH_FROM_ELMT((head)->hh.tbl, _hs_iter)->next));           \
} while (0)
#endif

#if _HASH_NONFATAL_OOM

#define _HASH_ADD_TO_TABLE(hh,head,keyptr,keylen_in,hashval,add,oomed)            \
do {                                                                             \
  if (!(oomed)) {                                                                \
    unsigned _ha_bkt;                                                            \
    (head)->hh.tbl->num_items++;                                                 \
    _HASH_TO_BKT(hashval, (head)->hh.tbl->num_buckets, _ha_bkt);                  \
    _HASH_ADD_TO_BKT((head)->hh.tbl->buckets[_ha_bkt], hh, &(add)->hh, oomed);    \
    if (oomed) {                                                                 \
      _HASH_ROLLBACK_BKT(hh, head, &(add)->hh);                                   \
      _HASH_DELETE_HH(hh, head, &(add)->hh);                                      \
      (add)->hh.tbl = NULL;                                                      \
      _uthash_nonfatal_oom(add);                                                  \
    } else {                                                                     \
      _HASH_BLOOM_ADD((head)->hh.tbl, hashval);                                   \
      _HASH_EMIT_KEY(hh, head, keyptr, keylen_in);                                \
    }                                                                            \
  } else {                                                                       \
    (add)->hh.tbl = NULL;                                                        \
    _uthash_nonfatal_oom(add);                                                    \
  }                                                                              \
} while (0)

#else

#define _HASH_ADD_TO_TABLE(hh,head,keyptr,keylen_in,hashval,add,oomed)            \
do {                                                                             \
  unsigned _ha_bkt;                                                              \
  (head)->hh.tbl->num_items++;                                                   \
  _HASH_TO_BKT(hashval, (head)->hh.tbl->num_buckets, _ha_bkt);                    \
  _HASH_ADD_TO_BKT((head)->hh.tbl->buckets[_ha_bkt], hh, &(add)->hh, oomed);      \
  _HASH_BLOOM_ADD((head)->hh.tbl, hashval);                                       \
  _HASH_EMIT_KEY(hh, head, keyptr, keylen_in);                                    \
} while (0)

#endif


#define _HASH_ADD_KEYPTR_BYHASHVALUE_INORDER(hh,head,keyptr,keylen_in,hashval,add,cmpfcn) \
do {                                                                             \
  _IF_HASH_NONFATAL_OOM( int _ha_oomed = 0; )                                     \
  (add)->hh.hashv = (hashval);                                                   \
  (add)->hh.key = (char*) (keyptr);                                              \
  (add)->hh.keylen = (unsigned) (keylen_in);                                     \
  if (!(head)) {                                                                 \
    (add)->hh.next = NULL;                                                       \
    (add)->hh.prev = NULL;                                                       \
    _HASH_MAKE_TABLE(hh, add, _ha_oomed);                                         \
    _IF_HASH_NONFATAL_OOM( if (!_ha_oomed) { )                                    \
      (head) = (add);                                                            \
    _IF_HASH_NONFATAL_OOM( } )                                                    \
  } else {                                                                       \
    void *_hs_iter = (head);                                                     \
    (add)->hh.tbl = (head)->hh.tbl;                                              \
    _HASH_AKBI_INNER_LOOP(hh, head, add, cmpfcn);                                 \
    if (_hs_iter) {                                                              \
      (add)->hh.next = _hs_iter;                                                 \
      if (((add)->hh.prev = _HH_FROM_ELMT((head)->hh.tbl, _hs_iter)->prev)) {     \
        _HH_FROM_ELMT((head)->hh.tbl, (add)->hh.prev)->next = (add);              \
      } else {                                                                   \
        (head) = (add);                                                          \
      }                                                                          \
      _HH_FROM_ELMT((head)->hh.tbl, _hs_iter)->prev = (add);                      \
    } else {                                                                     \
      _HASH_APPEND_LIST(hh, head, add);                                           \
    }                                                                            \
  }                                                                              \
  _HASH_ADD_TO_TABLE(hh, head, keyptr, keylen_in, hashval, add, _ha_oomed);       \
  _HASH_FSCK(hh, head, "_HASH_ADD_KEYPTR_BYHASHVALUE_INORDER");                    \
} while (0)

#define _HASH_ADD_KEYPTR_INORDER(hh,head,keyptr,keylen_in,add,cmpfcn)             \
do {                                                                             \
  unsigned _hs_hashv;                                                            \
  _HASH_VALUE(keyptr, keylen_in, _hs_hashv);                                      \
  _HASH_ADD_KEYPTR_BYHASHVALUE_INORDER(hh, head, keyptr, keylen_in, _hs_hashv, add, cmpfcn); \
} while (0)

#define _HASH_ADD_BYHASHVALUE_INORDER(hh,head,fieldname,keylen_in,hashval,add,cmpfcn) \
  _HASH_ADD_KEYPTR_BYHASHVALUE_INORDER(hh, head, &((add)->fieldname), keylen_in, hashval, add, cmpfcn)

#define _HASH_ADD_INORDER(hh,head,fieldname,keylen_in,add,cmpfcn)                 \
  _HASH_ADD_KEYPTR_INORDER(hh, head, &((add)->fieldname), keylen_in, add, cmpfcn)

#define _HASH_ADD_KEYPTR_BYHASHVALUE(hh,head,keyptr,keylen_in,hashval,add)        \
do {                                                                             \
  _IF_HASH_NONFATAL_OOM( int _ha_oomed = 0; )                                     \
  (add)->hh.hashv = (hashval);                                                   \
  (add)->hh.key = (const void*) (keyptr);                                        \
  (add)->hh.keylen = (unsigned) (keylen_in);                                     \
  if (!(head)) {                                                                 \
    (add)->hh.next = NULL;                                                       \
    (add)->hh.prev = NULL;                                                       \
    _HASH_MAKE_TABLE(hh, add, _ha_oomed);                                         \
    _IF_HASH_NONFATAL_OOM( if (!_ha_oomed) { )                                    \
      (head) = (add);                                                            \
    _IF_HASH_NONFATAL_OOM( } )                                                    \
  } else {                                                                       \
    (add)->hh.tbl = (head)->hh.tbl;                                              \
    _HASH_APPEND_LIST(hh, head, add);                                             \
  }                                                                              \
  _HASH_ADD_TO_TABLE(hh, head, keyptr, keylen_in, hashval, add, _ha_oomed);       \
  _HASH_FSCK(hh, head, "_HASH_ADD_KEYPTR_BYHASHVALUE");                            \
} while (0)

#define _HASH_ADD_KEYPTR(hh,head,keyptr,keylen_in,add)                            \
do {                                                                             \
  unsigned _ha_hashv;                                                            \
  _HASH_VALUE(keyptr, keylen_in, _ha_hashv);                                      \
  _HASH_ADD_KEYPTR_BYHASHVALUE(hh, head, keyptr, keylen_in, _ha_hashv, add);      \
} while (0)

#define _HASH_ADD_BYHASHVALUE(hh,head,fieldname,keylen_in,hashval,add)            \
  _HASH_ADD_KEYPTR_BYHASHVALUE(hh, head, &((add)->fieldname), keylen_in, hashval, add)

#define _HASH_ADD(hh,head,fieldname,keylen_in,add)                                \
  _HASH_ADD_KEYPTR(hh, head, &((add)->fieldname), keylen_in, add)

#define _HASH_TO_BKT(hashv,num_bkts,bkt)                                          \
do {                                                                             \
  bkt = ((hashv) & ((num_bkts) - 1U));                                           \
} while (0)

/* delete "delptr" from the hash table.
 * "the usual" patch-up process for the app-order doubly-linked-list.
 * The use of _hd_hh_del below deserves special explanation.
 * These used to be expressed using (delptr) but that led to a bug
 * if someone used the same symbol for the head and deletee, like
 *  _HASH_DELETE(hh,users,users);
 * We want that to work, but by changing the head (users) below
 * we were forfeiting our ability to further refer to the deletee (users)
 * in the patch-up process. Solution: use scratch space to
 * copy the deletee pointer, then the latter references are via that
 * scratch pointer rather than through the repointed (users) symbol.
 */
#define _HASH_DELETE(hh,head,delptr)                                              \
    _HASH_DELETE_HH(hh, head, &(delptr)->hh)

#define _HASH_DELETE_HH(hh,head,delptrhh)                                         \
do {                                                                             \
  const struct _mys_UT_hash_handle *_hd_hh_del = (delptrhh);                          \
  if ((_hd_hh_del->prev == NULL) && (_hd_hh_del->next == NULL)) {                \
    _HASH_BLOOM_FREE((head)->hh.tbl);                                             \
    _uthash_free((head)->hh.tbl->buckets,                                         \
                (head)->hh.tbl->num_buckets * sizeof(struct _mys_UT_hash_bucket));    \
    _uthash_free((head)->hh.tbl, sizeof(_mys_UT_hash_table));                          \
    (head) = NULL;                                                               \
  } else {                                                                       \
    unsigned _hd_bkt;                                                            \
    if (_hd_hh_del == (head)->hh.tbl->tail) {                                    \
      (head)->hh.tbl->tail = _HH_FROM_ELMT((head)->hh.tbl, _hd_hh_del->prev);     \
    }                                                                            \
    if (_hd_hh_del->prev != NULL) {                                              \
      _HH_FROM_ELMT((head)->hh.tbl, _hd_hh_del->prev)->next = _hd_hh_del->next;   \
    } else {                                                                     \
      _DECLTYPE_ASSIGN(head, _hd_hh_del->next);                                   \
    }                                                                            \
    if (_hd_hh_del->next != NULL) {                                              \
      _HH_FROM_ELMT((head)->hh.tbl, _hd_hh_del->next)->prev = _hd_hh_del->prev;   \
    }                                                                            \
    _HASH_TO_BKT(_hd_hh_del->hashv, (head)->hh.tbl->num_buckets, _hd_bkt);        \
    _HASH_DEL_IN_BKT((head)->hh.tbl->buckets[_hd_bkt], _hd_hh_del);               \
    (head)->hh.tbl->num_items--;                                                 \
  }                                                                              \
  _HASH_FSCK(hh, head, "_HASH_DELETE_HH");                                         \
} while (0)

/* convenience forms of _HASH_FIND/_HASH_ADD/_HASH_DEL */
#define _HASH_FIND_STR(head,findstr,out)                                          \
do {                                                                             \
    unsigned _uthash_hfstr_keylen = (unsigned)_uthash_strlen(findstr);            \
    _HASH_FIND(hh, head, findstr, _uthash_hfstr_keylen, out);                     \
} while (0)
#define _HASH_ADD_STR(head,strfield,add)                                          \
do {                                                                             \
    unsigned _uthash_hastr_keylen = (unsigned)_uthash_strlen((add)->strfield);    \
    _HASH_ADD(hh, head, strfield[0], _uthash_hastr_keylen, add);                  \
} while (0)
#define _HASH_REPLACE_STR(head,strfield,add,replaced)                             \
do {                                                                             \
    unsigned _uthash_hrstr_keylen = (unsigned)_uthash_strlen((add)->strfield);    \
    _HASH_REPLACE(hh, head, strfield[0], _uthash_hrstr_keylen, add, replaced);    \
} while (0)
#define _HASH_FIND_INT(head,findint,out)                                          \
    _HASH_FIND(hh,head,findint,sizeof(int),out)
#define _HASH_ADD_INT(head,intfield,add)                                          \
    _HASH_ADD(hh,head,intfield,sizeof(int),add)
#define _HASH_REPLACE_INT(head,intfield,add,replaced)                             \
    _HASH_REPLACE(hh,head,intfield,sizeof(int),add,replaced)
#define _HASH_FIND_PTR(head,findptr,out)                                          \
    _HASH_FIND(hh,head,findptr,sizeof(void *),out)
#define _HASH_ADD_PTR(head,ptrfield,add)                                          \
    _HASH_ADD(hh,head,ptrfield,sizeof(void *),add)
#define _HASH_REPLACE_PTR(head,ptrfield,add,replaced)                             \
    _HASH_REPLACE(hh,head,ptrfield,sizeof(void *),add,replaced)
#define _HASH_DEL(head,delptr)                                                    \
    _HASH_DELETE(hh,head,delptr)

/* _HASH_FSCK checks hash integrity on every add/delete when HASH_DEBUG is defined.
 * This is for uthash developer only; it compiles away if HASH_DEBUG isn't defined.
 */
#ifdef HASH_DEBUG
#include <stdio.h>   /* fprintf, stderr */
#define _HASH_OOPS(...) do { fprintf(stderr, __VA_ARGS__); exit(-1); } while (0)
#define _HASH_FSCK(hh,head,where)                                                 \
do {                                                                             \
  struct _mys_UT_hash_handle *_thh;                                                   \
  if (head) {                                                                    \
    unsigned _bkt_i;                                                             \
    unsigned _count = 0;                                                         \
    char *_prev;                                                                 \
    for (_bkt_i = 0; _bkt_i < (head)->hh.tbl->num_buckets; ++_bkt_i) {           \
      unsigned _bkt_count = 0;                                                   \
      _thh = (head)->hh.tbl->buckets[_bkt_i].hh_head;                            \
      _prev = NULL;                                                              \
      while (_thh) {                                                             \
        if (_prev != (char*)(_thh->hh_prev)) {                                   \
          _HASH_OOPS("%s: invalid hh_prev %p, actual %p\n",                       \
              (where), (void*)_thh->hh_prev, (void*)_prev);                      \
        }                                                                        \
        _bkt_count++;                                                            \
        _prev = (char*)(_thh);                                                   \
        _thh = _thh->hh_next;                                                    \
      }                                                                          \
      _count += _bkt_count;                                                      \
      if ((head)->hh.tbl->buckets[_bkt_i].count !=  _bkt_count) {                \
        _HASH_OOPS("%s: invalid bucket count %u, actual %u\n",                    \
            (where), (head)->hh.tbl->buckets[_bkt_i].count, _bkt_count);         \
      }                                                                          \
    }                                                                            \
    if (_count != (head)->hh.tbl->num_items) {                                   \
      _HASH_OOPS("%s: invalid hh item count %u, actual %u\n",                     \
          (where), (head)->hh.tbl->num_items, _count);                           \
    }                                                                            \
    _count = 0;                                                                  \
    _prev = NULL;                                                                \
    _thh =  &(head)->hh;                                                         \
    while (_thh) {                                                               \
      _count++;                                                                  \
      if (_prev != (char*)_thh->prev) {                                          \
        _HASH_OOPS("%s: invalid prev %p, actual %p\n",                            \
            (where), (void*)_thh->prev, (void*)_prev);                           \
      }                                                                          \
      _prev = (char*)_ELMT_FROM_HH((head)->hh.tbl, _thh);                         \
      _thh = (_thh->next ? _HH_FROM_ELMT((head)->hh.tbl, _thh->next) : NULL);     \
    }                                                                            \
    if (_count != (head)->hh.tbl->num_items) {                                   \
      _HASH_OOPS("%s: invalid app item count %u, actual %u\n",                    \
          (where), (head)->hh.tbl->num_items, _count);                           \
    }                                                                            \
  }                                                                              \
} while (0)
#else
#define _HASH_FSCK(hh,head,where)
#endif

/* When compiled with -DHASH_EMIT_KEYS, length-prefixed keys are emitted to
 * the descriptor to which this macro is defined for tuning the hash function.
 * The app can #include <unistd.h> to get the prototype for write(2). */
#ifdef HASH_EMIT_KEYS
#define _HASH_EMIT_KEY(hh,head,keyptr,fieldlen)                                   \
do {                                                                             \
  unsigned _klen = fieldlen;                                                     \
  write(HASH_EMIT_KEYS, &_klen, sizeof(_klen));                                  \
  write(HASH_EMIT_KEYS, keyptr, (unsigned long)fieldlen);                        \
} while (0)
#else
#define _HASH_EMIT_KEY(hh,head,keyptr,fieldlen)
#endif

/* The Bernstein hash function, used in Perl prior to v5.6. Note (x<<5+x)=x*33. */
#define _HASH_BER(key,keylen,hashv)                                               \
do {                                                                             \
  unsigned _hb_keylen = (unsigned)keylen;                                        \
  const unsigned char *_hb_key = (const unsigned char*)(key);                    \
  (hashv) = 0;                                                                   \
  while (_hb_keylen-- != 0U) {                                                   \
    (hashv) = (((hashv) << 5) + (hashv)) + *_hb_key++;                           \
  }                                                                              \
} while (0)


/* SAX/FNV/OAT/JEN hash functions are macro variants of those listed at
 * http://eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx
 * (archive link: https://archive.is/Ivcan )
 */
#define _HASH_SAX(key,keylen,hashv)                                               \
do {                                                                             \
  unsigned _sx_i;                                                                \
  const unsigned char *_hs_key = (const unsigned char*)(key);                    \
  hashv = 0;                                                                     \
  for (_sx_i=0; _sx_i < keylen; _sx_i++) {                                       \
    hashv ^= (hashv << 5) + (hashv >> 2) + _hs_key[_sx_i];                       \
  }                                                                              \
} while (0)
/* FNV-1a variation */
#define _HASH_FNV(key,keylen,hashv)                                               \
do {                                                                             \
  unsigned _fn_i;                                                                \
  const unsigned char *_hf_key = (const unsigned char*)(key);                    \
  (hashv) = 2166136261U;                                                         \
  for (_fn_i=0; _fn_i < keylen; _fn_i++) {                                       \
    hashv = hashv ^ _hf_key[_fn_i];                                              \
    hashv = hashv * 16777619U;                                                   \
  }                                                                              \
} while (0)

#define _HASH_OAT(key,keylen,hashv)                                               \
do {                                                                             \
  unsigned _ho_i;                                                                \
  const unsigned char *_ho_key=(const unsigned char*)(key);                      \
  hashv = 0;                                                                     \
  for(_ho_i=0; _ho_i < keylen; _ho_i++) {                                        \
      hashv += _ho_key[_ho_i];                                                   \
      hashv += (hashv << 10);                                                    \
      hashv ^= (hashv >> 6);                                                     \
  }                                                                              \
  hashv += (hashv << 3);                                                         \
  hashv ^= (hashv >> 11);                                                        \
  hashv += (hashv << 15);                                                        \
} while (0)

#define _HASH_JEN_MIX(a,b,c)                                                      \
do {                                                                             \
  a -= b; a -= c; a ^= ( c >> 13 );                                              \
  b -= c; b -= a; b ^= ( a << 8 );                                               \
  c -= a; c -= b; c ^= ( b >> 13 );                                              \
  a -= b; a -= c; a ^= ( c >> 12 );                                              \
  b -= c; b -= a; b ^= ( a << 16 );                                              \
  c -= a; c -= b; c ^= ( b >> 5 );                                               \
  a -= b; a -= c; a ^= ( c >> 3 );                                               \
  b -= c; b -= a; b ^= ( a << 10 );                                              \
  c -= a; c -= b; c ^= ( b >> 15 );                                              \
} while (0)

#define _HASH_JEN(key,keylen,hashv)                                               \
do {                                                                             \
  unsigned _hj_i,_hj_j,_hj_k;                                                    \
  unsigned const char *_hj_key=(unsigned const char*)(key);                      \
  hashv = 0xfeedbeefu;                                                           \
  _hj_i = _hj_j = 0x9e3779b9u;                                                   \
  _hj_k = (unsigned)(keylen);                                                    \
  while (_hj_k >= 12U) {                                                         \
    _hj_i +=    (_hj_key[0] + ( (unsigned)_hj_key[1] << 8 )                      \
        + ( (unsigned)_hj_key[2] << 16 )                                         \
        + ( (unsigned)_hj_key[3] << 24 ) );                                      \
    _hj_j +=    (_hj_key[4] + ( (unsigned)_hj_key[5] << 8 )                      \
        + ( (unsigned)_hj_key[6] << 16 )                                         \
        + ( (unsigned)_hj_key[7] << 24 ) );                                      \
    hashv += (_hj_key[8] + ( (unsigned)_hj_key[9] << 8 )                         \
        + ( (unsigned)_hj_key[10] << 16 )                                        \
        + ( (unsigned)_hj_key[11] << 24 ) );                                     \
                                                                                 \
     _HASH_JEN_MIX(_hj_i, _hj_j, hashv);                                          \
                                                                                 \
     _hj_key += 12;                                                              \
     _hj_k -= 12U;                                                               \
  }                                                                              \
  hashv += (unsigned)(keylen);                                                   \
  switch ( _hj_k ) {                                                             \
    case 11: hashv += ( (unsigned)_hj_key[10] << 24 ); /* FALLTHROUGH */         \
    case 10: hashv += ( (unsigned)_hj_key[9] << 16 );  /* FALLTHROUGH */         \
    case 9:  hashv += ( (unsigned)_hj_key[8] << 8 );   /* FALLTHROUGH */         \
    case 8:  _hj_j += ( (unsigned)_hj_key[7] << 24 );  /* FALLTHROUGH */         \
    case 7:  _hj_j += ( (unsigned)_hj_key[6] << 16 );  /* FALLTHROUGH */         \
    case 6:  _hj_j += ( (unsigned)_hj_key[5] << 8 );   /* FALLTHROUGH */         \
    case 5:  _hj_j += _hj_key[4];                      /* FALLTHROUGH */         \
    case 4:  _hj_i += ( (unsigned)_hj_key[3] << 24 );  /* FALLTHROUGH */         \
    case 3:  _hj_i += ( (unsigned)_hj_key[2] << 16 );  /* FALLTHROUGH */         \
    case 2:  _hj_i += ( (unsigned)_hj_key[1] << 8 );   /* FALLTHROUGH */         \
    case 1:  _hj_i += _hj_key[0];                      /* FALLTHROUGH */         \
    default: ;                                                                   \
  }                                                                              \
  _HASH_JEN_MIX(_hj_i, _hj_j, hashv);                                             \
} while (0)

/* The Paul Hsieh hash function */
#undef _get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__)             \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define _get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (_get16bits)
#define _get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)             \
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif
#define _HASH_SFH(key,keylen,hashv)                                               \
do {                                                                             \
  unsigned const char *_sfh_key=(unsigned const char*)(key);                     \
  uint32_t _sfh_tmp, _sfh_len = (uint32_t)keylen;                                \
                                                                                 \
  unsigned _sfh_rem = _sfh_len & 3U;                                             \
  _sfh_len >>= 2;                                                                \
  hashv = 0xcafebabeu;                                                           \
                                                                                 \
  /* Main loop */                                                                \
  for (;_sfh_len > 0U; _sfh_len--) {                                             \
    hashv    += _get16bits (_sfh_key);                                            \
    _sfh_tmp  = ((uint32_t)(_get16bits (_sfh_key+2)) << 11) ^ hashv;              \
    hashv     = (hashv << 16) ^ _sfh_tmp;                                        \
    _sfh_key += 2U*sizeof (uint16_t);                                            \
    hashv    += hashv >> 11;                                                     \
  }                                                                              \
                                                                                 \
  /* Handle end cases */                                                         \
  switch (_sfh_rem) {                                                            \
    case 3: hashv += _get16bits (_sfh_key);                                       \
            hashv ^= hashv << 16;                                                \
            hashv ^= (uint32_t)(_sfh_key[sizeof (uint16_t)]) << 18;              \
            hashv += hashv >> 11;                                                \
            break;                                                               \
    case 2: hashv += _get16bits (_sfh_key);                                       \
            hashv ^= hashv << 11;                                                \
            hashv += hashv >> 17;                                                \
            break;                                                               \
    case 1: hashv += *_sfh_key;                                                  \
            hashv ^= hashv << 10;                                                \
            hashv += hashv >> 1;                                                 \
            break;                                                               \
    default: ;                                                                   \
  }                                                                              \
                                                                                 \
  /* Force "avalanching" of final 127 bits */                                    \
  hashv ^= hashv << 3;                                                           \
  hashv += hashv >> 5;                                                           \
  hashv ^= hashv << 4;                                                           \
  hashv += hashv >> 17;                                                          \
  hashv ^= hashv << 25;                                                          \
  hashv += hashv >> 6;                                                           \
} while (0)

/* iterate over items in a known bucket to find desired item */
#define _HASH_FIND_IN_BKT(tbl,hh,head,keyptr,keylen_in,hashval,out)               \
do {                                                                             \
  if ((head).hh_head != NULL) {                                                  \
    _DECLTYPE_ASSIGN(out, _ELMT_FROM_HH(tbl, (head).hh_head));                     \
  } else {                                                                       \
    (out) = NULL;                                                                \
  }                                                                              \
  while ((out) != NULL) {                                                        \
    if ((out)->hh.hashv == (hashval) && (out)->hh.keylen == (keylen_in)) {       \
      if (_HASH_KEYCMP((out)->hh.key, keyptr, keylen_in) == 0) {                  \
        break;                                                                   \
      }                                                                          \
    }                                                                            \
    if ((out)->hh.hh_next != NULL) {                                             \
      _DECLTYPE_ASSIGN(out, _ELMT_FROM_HH(tbl, (out)->hh.hh_next));                \
    } else {                                                                     \
      (out) = NULL;                                                              \
    }                                                                            \
  }                                                                              \
} while (0)

/* add an item to a bucket  */
#define _HASH_ADD_TO_BKT(head,hh,addhh,oomed)                                     \
do {                                                                             \
  _mys_UT_hash_bucket *_ha_head = &(head);                                            \
  _ha_head->count++;                                                             \
  (addhh)->hh_next = _ha_head->hh_head;                                          \
  (addhh)->hh_prev = NULL;                                                       \
  if (_ha_head->hh_head != NULL) {                                               \
    _ha_head->hh_head->hh_prev = (addhh);                                        \
  }                                                                              \
  _ha_head->hh_head = (addhh);                                                   \
  if ((_ha_head->count >= ((_ha_head->expand_mult + 1U) * _HASH_BKT_CAPACITY_THRESH)) \
      && !(addhh)->tbl->noexpand) {                                              \
    _HASH_EXPAND_BUCKETS(addhh,(addhh)->tbl, oomed);                              \
    _IF_HASH_NONFATAL_OOM(                                                        \
      if (oomed) {                                                               \
        _HASH_DEL_IN_BKT(head,addhh);                                             \
      }                                                                          \
    )                                                                            \
  }                                                                              \
} while (0)

/* remove an item from a given bucket */
#define _HASH_DEL_IN_BKT(head,delhh)                                              \
do {                                                                             \
  _mys_UT_hash_bucket *_hd_head = &(head);                                            \
  _hd_head->count--;                                                             \
  if (_hd_head->hh_head == (delhh)) {                                            \
    _hd_head->hh_head = (delhh)->hh_next;                                        \
  }                                                                              \
  if ((delhh)->hh_prev) {                                                        \
    (delhh)->hh_prev->hh_next = (delhh)->hh_next;                                \
  }                                                                              \
  if ((delhh)->hh_next) {                                                        \
    (delhh)->hh_next->hh_prev = (delhh)->hh_prev;                                \
  }                                                                              \
} while (0)

/* Bucket expansion has the effect of doubling the number of buckets
 * and redistributing the items into the new buckets. Ideally the
 * items will distribute more or less evenly into the new buckets
 * (the extent to which this is true is a measure of the quality of
 * the hash function as it applies to the key domain).
 *
 * With the items distributed into more buckets, the chain length
 * (item count) in each bucket is reduced. Thus by expanding buckets
 * the hash keeps a bound on the chain length. This bounded chain
 * length is the essence of how a hash provides constant time lookup.
 *
 * The calculation of tbl->ideal_chain_maxlen below deserves some
 * explanation. First, keep in mind that we're calculating the ideal
 * maximum chain length based on the *new* (doubled) bucket count.
 * In fractions this is just n/b (n=number of items,b=new num buckets).
 * Since the ideal chain length is an integer, we want to calculate
 * ceil(n/b). We don't depend on floating point arithmetic in this
 * hash, so to calculate ceil(n/b) with integers we could write
 *
 *      ceil(n/b) = (n/b) + ((n%b)?1:0)
 *
 * and in fact a previous version of this hash did just that.
 * But now we have improved things a bit by recognizing that b is
 * always a power of two. We keep its base 2 log handy (call it lb),
 * so now we can write this with a bit shift and logical AND:
 *
 *      ceil(n/b) = (n>>lb) + ( (n & (b-1)) ? 1:0)
 *
 */
#define _HASH_EXPAND_BUCKETS(hh,tbl,oomed)                                        \
do {                                                                             \
  unsigned _he_bkt;                                                              \
  unsigned _he_bkt_i;                                                            \
  struct _mys_UT_hash_handle *_he_thh, *_he_hh_nxt;                                   \
  _mys_UT_hash_bucket *_he_new_buckets, *_he_newbkt;                                  \
  _he_new_buckets = (_mys_UT_hash_bucket*)_uthash_malloc(                              \
           sizeof(struct _mys_UT_hash_bucket) * (tbl)->num_buckets * 2U);             \
  if (!_he_new_buckets) {                                                        \
    _HASH_RECORD_OOM(oomed);                                                      \
  } else {                                                                       \
    _uthash_bzero(_he_new_buckets,                                                \
        sizeof(struct _mys_UT_hash_bucket) * (tbl)->num_buckets * 2U);                \
    (tbl)->ideal_chain_maxlen =                                                  \
       ((tbl)->num_items >> ((tbl)->log2_num_buckets+1U)) +                      \
       ((((tbl)->num_items & (((tbl)->num_buckets*2U)-1U)) != 0U) ? 1U : 0U);    \
    (tbl)->nonideal_items = 0;                                                   \
    for (_he_bkt_i = 0; _he_bkt_i < (tbl)->num_buckets; _he_bkt_i++) {           \
      _he_thh = (tbl)->buckets[ _he_bkt_i ].hh_head;                             \
      while (_he_thh != NULL) {                                                  \
        _he_hh_nxt = _he_thh->hh_next;                                           \
        _HASH_TO_BKT(_he_thh->hashv, (tbl)->num_buckets * 2U, _he_bkt);           \
        _he_newbkt = &(_he_new_buckets[_he_bkt]);                                \
        if (++(_he_newbkt->count) > (tbl)->ideal_chain_maxlen) {                 \
          (tbl)->nonideal_items++;                                               \
          if (_he_newbkt->count > _he_newbkt->expand_mult * (tbl)->ideal_chain_maxlen) { \
            _he_newbkt->expand_mult++;                                           \
          }                                                                      \
        }                                                                        \
        _he_thh->hh_prev = NULL;                                                 \
        _he_thh->hh_next = _he_newbkt->hh_head;                                  \
        if (_he_newbkt->hh_head != NULL) {                                       \
          _he_newbkt->hh_head->hh_prev = _he_thh;                                \
        }                                                                        \
        _he_newbkt->hh_head = _he_thh;                                           \
        _he_thh = _he_hh_nxt;                                                    \
      }                                                                          \
    }                                                                            \
    _uthash_free((tbl)->buckets, (tbl)->num_buckets * sizeof(struct _mys_UT_hash_bucket)); \
    (tbl)->num_buckets *= 2U;                                                    \
    (tbl)->log2_num_buckets++;                                                   \
    (tbl)->buckets = _he_new_buckets;                                            \
    (tbl)->ineff_expands = ((tbl)->nonideal_items > ((tbl)->num_items >> 1)) ?   \
        ((tbl)->ineff_expands+1U) : 0U;                                          \
    if ((tbl)->ineff_expands > 1U) {                                             \
      (tbl)->noexpand = 1;                                                       \
      _uthash_noexpand_fyi(tbl);                                                  \
    }                                                                            \
    _uthash_expand_fyi(tbl);                                                      \
  }                                                                              \
} while (0)


/* This is an adaptation of Simon Tatham's O(n log(n)) mergesort */
/* Note that _HASH_SORT assumes the hash handle name to be hh.
 * _HASH_SRT was added to allow the hash handle name to be passed in. */
#define _HASH_SORT(head,cmpfcn) _HASH_SRT(hh,head,cmpfcn)
#define _HASH_SRT(hh,head,cmpfcn)                                                 \
do {                                                                             \
  unsigned _hs_i;                                                                \
  unsigned _hs_looping,_hs_nmerges,_hs_insize,_hs_psize,_hs_qsize;               \
  struct _mys_UT_hash_handle *_hs_p, *_hs_q, *_hs_e, *_hs_list, *_hs_tail;            \
  if (head != NULL) {                                                            \
    _hs_insize = 1;                                                              \
    _hs_looping = 1;                                                             \
    _hs_list = &((head)->hh);                                                    \
    while (_hs_looping != 0U) {                                                  \
      _hs_p = _hs_list;                                                          \
      _hs_list = NULL;                                                           \
      _hs_tail = NULL;                                                           \
      _hs_nmerges = 0;                                                           \
      while (_hs_p != NULL) {                                                    \
        _hs_nmerges++;                                                           \
        _hs_q = _hs_p;                                                           \
        _hs_psize = 0;                                                           \
        for (_hs_i = 0; _hs_i < _hs_insize; ++_hs_i) {                           \
          _hs_psize++;                                                           \
          _hs_q = ((_hs_q->next != NULL) ?                                       \
            _HH_FROM_ELMT((head)->hh.tbl, _hs_q->next) : NULL);                   \
          if (_hs_q == NULL) {                                                   \
            break;                                                               \
          }                                                                      \
        }                                                                        \
        _hs_qsize = _hs_insize;                                                  \
        while ((_hs_psize != 0U) || ((_hs_qsize != 0U) && (_hs_q != NULL))) {    \
          if (_hs_psize == 0U) {                                                 \
            _hs_e = _hs_q;                                                       \
            _hs_q = ((_hs_q->next != NULL) ?                                     \
              _HH_FROM_ELMT((head)->hh.tbl, _hs_q->next) : NULL);                 \
            _hs_qsize--;                                                         \
          } else if ((_hs_qsize == 0U) || (_hs_q == NULL)) {                     \
            _hs_e = _hs_p;                                                       \
            if (_hs_p != NULL) {                                                 \
              _hs_p = ((_hs_p->next != NULL) ?                                   \
                _HH_FROM_ELMT((head)->hh.tbl, _hs_p->next) : NULL);               \
            }                                                                    \
            _hs_psize--;                                                         \
          } else if ((cmpfcn(                                                    \
                _DECLTYPE(head)(_ELMT_FROM_HH((head)->hh.tbl, _hs_p)),             \
                _DECLTYPE(head)(_ELMT_FROM_HH((head)->hh.tbl, _hs_q))              \
                )) <= 0) {                                                       \
            _hs_e = _hs_p;                                                       \
            if (_hs_p != NULL) {                                                 \
              _hs_p = ((_hs_p->next != NULL) ?                                   \
                _HH_FROM_ELMT((head)->hh.tbl, _hs_p->next) : NULL);               \
            }                                                                    \
            _hs_psize--;                                                         \
          } else {                                                               \
            _hs_e = _hs_q;                                                       \
            _hs_q = ((_hs_q->next != NULL) ?                                     \
              _HH_FROM_ELMT((head)->hh.tbl, _hs_q->next) : NULL);                 \
            _hs_qsize--;                                                         \
          }                                                                      \
          if ( _hs_tail != NULL ) {                                              \
            _hs_tail->next = ((_hs_e != NULL) ?                                  \
              _ELMT_FROM_HH((head)->hh.tbl, _hs_e) : NULL);                       \
          } else {                                                               \
            _hs_list = _hs_e;                                                    \
          }                                                                      \
          if (_hs_e != NULL) {                                                   \
            _hs_e->prev = ((_hs_tail != NULL) ?                                  \
              _ELMT_FROM_HH((head)->hh.tbl, _hs_tail) : NULL);                    \
          }                                                                      \
          _hs_tail = _hs_e;                                                      \
        }                                                                        \
        _hs_p = _hs_q;                                                           \
      }                                                                          \
      if (_hs_tail != NULL) {                                                    \
        _hs_tail->next = NULL;                                                   \
      }                                                                          \
      if (_hs_nmerges <= 1U) {                                                   \
        _hs_looping = 0;                                                         \
        (head)->hh.tbl->tail = _hs_tail;                                         \
        _DECLTYPE_ASSIGN(head, _ELMT_FROM_HH((head)->hh.tbl, _hs_list));           \
      }                                                                          \
      _hs_insize *= 2U;                                                          \
    }                                                                            \
    _HASH_FSCK(hh, head, "_HASH_SRT");                                             \
  }                                                                              \
} while (0)

/* This function selects items from one hash into another hash.
 * The end result is that the selected items have dual presence
 * in both hashes. There is no copy of the items made; rather
 * they are added into the new hash through a secondary hash
 * hash handle that must be present in the structure. */
#define _HASH_SELECT(hh_dst, dst, hh_src, src, cond)                              \
do {                                                                             \
  unsigned _src_bkt, _dst_bkt;                                                   \
  void *_last_elt = NULL, *_elt;                                                 \
  _mys_UT_hash_handle *_src_hh, *_dst_hh, *_last_elt_hh=NULL;                         \
  ptrdiff_t _dst_hho = ((char*)(&(dst)->hh_dst) - (char*)(dst));                 \
  if ((src) != NULL) {                                                           \
    for (_src_bkt=0; _src_bkt < (src)->hh_src.tbl->num_buckets; _src_bkt++) {    \
      for (_src_hh = (src)->hh_src.tbl->buckets[_src_bkt].hh_head;               \
        _src_hh != NULL;                                                         \
        _src_hh = _src_hh->hh_next) {                                            \
        _elt = _ELMT_FROM_HH((src)->hh_src.tbl, _src_hh);                         \
        if (cond(_elt)) {                                                        \
          _IF_HASH_NONFATAL_OOM( int _hs_oomed = 0; )                             \
          _dst_hh = (_mys_UT_hash_handle*)(void*)(((char*)_elt) + _dst_hho);          \
          _dst_hh->key = _src_hh->key;                                           \
          _dst_hh->keylen = _src_hh->keylen;                                     \
          _dst_hh->hashv = _src_hh->hashv;                                       \
          _dst_hh->prev = _last_elt;                                             \
          _dst_hh->next = NULL;                                                  \
          if (_last_elt_hh != NULL) {                                            \
            _last_elt_hh->next = _elt;                                           \
          }                                                                      \
          if ((dst) == NULL) {                                                   \
            _DECLTYPE_ASSIGN(dst, _elt);                                          \
            _HASH_MAKE_TABLE(hh_dst, dst, _hs_oomed);                             \
            _IF_HASH_NONFATAL_OOM(                                                \
              if (_hs_oomed) {                                                   \
                _uthash_nonfatal_oom(_elt);                                       \
                (dst) = NULL;                                                    \
                continue;                                                        \
              }                                                                  \
            )                                                                    \
          } else {                                                               \
            _dst_hh->tbl = (dst)->hh_dst.tbl;                                    \
          }                                                                      \
          _HASH_TO_BKT(_dst_hh->hashv, _dst_hh->tbl->num_buckets, _dst_bkt);      \
          _HASH_ADD_TO_BKT(_dst_hh->tbl->buckets[_dst_bkt], hh_dst, _dst_hh, _hs_oomed); \
          (dst)->hh_dst.tbl->num_items++;                                        \
          _IF_HASH_NONFATAL_OOM(                                                  \
            if (_hs_oomed) {                                                     \
              _HASH_ROLLBACK_BKT(hh_dst, dst, _dst_hh);                           \
              _HASH_DELETE_HH(hh_dst, dst, _dst_hh);                              \
              _dst_hh->tbl = NULL;                                               \
              _uthash_nonfatal_oom(_elt);                                         \
              continue;                                                          \
            }                                                                    \
          )                                                                      \
          _HASH_BLOOM_ADD(_dst_hh->tbl, _dst_hh->hashv);                          \
          _last_elt = _elt;                                                      \
          _last_elt_hh = _dst_hh;                                                \
        }                                                                        \
      }                                                                          \
    }                                                                            \
  }                                                                              \
  _HASH_FSCK(hh_dst, dst, "_HASH_SELECT");                                         \
} while (0)

#define _HASH_CLEAR(hh,head)                                                      \
do {                                                                             \
  if ((head) != NULL) {                                                          \
    _HASH_BLOOM_FREE((head)->hh.tbl);                                             \
    _uthash_free((head)->hh.tbl->buckets,                                         \
                (head)->hh.tbl->num_buckets*sizeof(struct _mys_UT_hash_bucket));      \
    _uthash_free((head)->hh.tbl, sizeof(_mys_UT_hash_table));                          \
    (head) = NULL;                                                               \
  }                                                                              \
} while (0)

#define _HASH_OVERHEAD(hh,head)                                                   \
 (((head) != NULL) ? (                                                           \
 (size_t)(((head)->hh.tbl->num_items   * sizeof(_mys_UT_hash_handle))   +             \
          ((head)->hh.tbl->num_buckets * sizeof(_mys_UT_hash_bucket))   +             \
           sizeof(_mys_UT_hash_table)                                   +             \
           (_HASH_BLOOM_BYTELEN))) : 0U)

#ifdef _NO_DECLTYPE
#define _HASH_ITER(hh,head,el,tmp)                                                \
for(((el)=(head)), ((*(char**)(&(tmp)))=(char*)((head!=NULL)?(head)->hh.next:NULL)); \
  (el) != NULL; ((el)=(tmp)), ((*(char**)(&(tmp)))=(char*)((tmp!=NULL)?(tmp)->hh.next:NULL)))
#else
#define _HASH_ITER(hh,head,el,tmp)                                                \
for(((el)=(head)), ((tmp)=_DECLTYPE(el)((head!=NULL)?(head)->hh.next:NULL));      \
  (el) != NULL; ((el)=(tmp)), ((tmp)=_DECLTYPE(el)((tmp!=NULL)?(tmp)->hh.next:NULL)))
#endif

/* obtain a count of items in the hash */
#define _HASH_COUNT(head) _HASH_CNT(hh,head)
#define _HASH_CNT(hh,head) ((head != NULL)?((head)->hh.tbl->num_items):0U)

/* random signature used only to find hash tables in external analysis */
#define _HASH_SIGNATURE 0xa0111fe1u
#define _HASH_BLOOM_SIGNATURE 0xb12220f2u

#endif /* _UTHASH_H */
#endif /*_UTHASH_DEFINE*/



/************************************************/
/************************************************/
// [Part 2] Define struct type
/************************************************/
/************************************************/
#ifndef _UTHASH_H_TYPE
#define _UTHASH_H_TYPE
typedef struct _mys_UT_hash_bucket {
   struct _mys_UT_hash_handle *hh_head;
   unsigned count;

   /* expand_mult is normally set to 0. In this situation, the max chain length
    * threshold is enforced at its default value, _HASH_BKT_CAPACITY_THRESH. (If
    * the bucket's chain exceeds this length, bucket expansion is triggered).
    * However, setting expand_mult to a non-zero value delays bucket expansion
    * (that would be triggered by additions to this particular bucket)
    * until its chain length reaches a *multiple* of _HASH_BKT_CAPACITY_THRESH.
    * (The multiplier is simply expand_mult+1). The whole idea of this
    * multiplier is to reduce bucket expansions, since they are expensive, in
    * situations where we know that a particular bucket tends to be overused.
    * It is better to let its chain length grow to a longer yet-still-bounded
    * value, than to do an O(n) bucket expansion too often.
    */
   unsigned expand_mult;

} _mys_UT_hash_bucket;

typedef struct _mys_UT_hash_table {
   _mys_UT_hash_bucket *buckets;
   unsigned num_buckets, log2_num_buckets;
   unsigned num_items;
   struct _mys_UT_hash_handle *tail; /* tail hh in app order, for fast append    */
   ptrdiff_t hho; /* hash handle offset (byte pos of hash handle in element */

   /* in an ideal situation (all buckets used equally), no bucket would have
    * more than ceil(#items/#buckets) items. that's the ideal chain length. */
   unsigned ideal_chain_maxlen;

   /* nonideal_items is the number of items in the hash whose chain position
    * exceeds the ideal chain maxlen. these items pay the penalty for an uneven
    * hash distribution; reaching them in a chain traversal takes >ideal steps */
   unsigned nonideal_items;

   /* ineffective expands occur when a bucket doubling was performed, but
    * afterward, more than half the items in the hash had nonideal chain
    * positions. If this happens on two consecutive expansions we inhibit any
    * further expansion, as it's not helping; this happens when the hash
    * function isn't a good fit for the key domain. When expansion is inhibited
    * the hash will still work, albeit no longer in constant time. */
   unsigned ineff_expands, noexpand;

   uint32_t signature; /* used only to find hash tables in external analysis */
#ifdef _HASH_BLOOM
   uint32_t bloom_sig; /* used only to test bloom exists in external analysis */
   uint8_t *bloom_bv;
   uint8_t bloom_nbits;
#endif

} _mys_UT_hash_table;

typedef struct _mys_UT_hash_handle {
   struct _mys_UT_hash_table *tbl;
   void *prev;                       /* prev element in app order      */
   void *next;                       /* next element in app order      */
   struct _mys_UT_hash_handle *hh_prev;   /* previous hh in bucket order    */
   struct _mys_UT_hash_handle *hh_next;   /* next hh in bucket order        */
   const void *key;                  /* ptr to enclosing struct's key  */
   unsigned keylen;                  /* enclosing struct's key len     */
   unsigned hashv;                   /* result of hash-fcn(key)        */
} _mys_UT_hash_handle;

#endif /* _UTHASH_H_TYPE */



/************************************************/
/************************************************/
// [Part 3] Undef all these things if you want
/************************************************/
/************************************************/
#ifdef _UTHASH_UNDEF
#undef _UTHASH_UNDEF
#undef _UTHASH_H
#undef _UTHASH_VERSION
#undef _DECLTYPE
#undef _NO_DECLTYPE
#undef _DECLTYPE
#undef _NO_DECLTYPE
#undef _DECLTYPE
#undef _DECLTYPE
#undef _DECLTYPE_ASSIGN
#undef _DECLTYPE_ASSIGN
#undef _uthash_malloc
#undef _uthash_free
#undef _uthash_bzero
#undef _uthash_strlen
#undef _HASH_FUNCTION
#undef _HASH_KEYCMP
#undef _uthash_noexpand_fyi
#undef _uthash_expand_fyi
#undef _HASH_NONFATAL_OOM
#undef _uthash_nonfatal_oom
#undef _HASH_RECORD_OOM
#undef _IF_HASH_NONFATAL_OOM
#undef _uthash_fatal
#undef _HASH_RECORD_OOM
#undef _IF_HASH_NONFATAL_OOM
#undef _HASH_INITIAL_NUM_BUCKETS
#undef _HASH_INITIAL_NUM_BUCKETS_LOG2
#undef _HASH_BKT_CAPACITY_THRESH
#undef _ELMT_FROM_HH
#undef _HH_FROM_ELMT
#undef _HASH_ROLLBACK_BKT
#undef _HASH_VALUE
#undef _HASH_FIND_BYHASHVALUE
#undef _HASH_FIND
#undef _HASH_BLOOM_BITLEN
#undef _HASH_BLOOM_BYTELEN
#undef _HASH_BLOOM_MAKE
#undef _HASH_BLOOM_FREE
#undef _HASH_BLOOM_BITSET
#undef _HASH_BLOOM_BITTEST
#undef _HASH_BLOOM_ADD
#undef _HASH_BLOOM_TEST
#undef _HASH_BLOOM_MAKE
#undef _HASH_BLOOM_FREE
#undef _HASH_BLOOM_ADD
#undef _HASH_BLOOM_TEST
#undef _HASH_BLOOM_BYTELEN
#undef _HASH_MAKE_TABLE
#undef _HASH_REPLACE_BYHASHVALUE_INORDER
#undef _HASH_REPLACE_BYHASHVALUE
#undef _HASH_REPLACE
#undef _HASH_REPLACE_INORDER
#undef _HASH_APPEND_LIST
#undef _HASH_AKBI_INNER_LOOP
#undef _HASH_AKBI_INNER_LOOP
#undef _HASH_ADD_TO_TABLE
#undef _HASH_ADD_TO_TABLE
#undef _HASH_ADD_KEYPTR_BYHASHVALUE_INORDER
#undef _HASH_ADD_KEYPTR_INORDER
#undef _HASH_ADD_BYHASHVALUE_INORDER
#undef _HASH_ADD_INORDER
#undef _HASH_ADD_KEYPTR_BYHASHVALUE
#undef _HASH_ADD_KEYPTR
#undef _HASH_ADD_BYHASHVALUE
#undef _HASH_ADD
#undef _HASH_TO_BKT
#undef _HASH_DELETE
#undef _HASH_DELETE_HH
#undef _HASH_FIND_STR
#undef _HASH_ADD_STR
#undef _HASH_REPLACE_STR
#undef _HASH_FIND_INT
#undef _HASH_ADD_INT
#undef _HASH_REPLACE_INT
#undef _HASH_FIND_PTR
#undef _HASH_ADD_PTR
#undef _HASH_REPLACE_PTR
#undef _HASH_DEL
#undef _HASH_OOPS
#undef _HASH_FSCK
#undef _HASH_FSCK
#undef _HASH_EMIT_KEY
#undef _HASH_EMIT_KEY
#undef _HASH_BER
#undef _HASH_SAX
#undef _HASH_FNV
#undef _HASH_OAT
#undef _HASH_JEN_MIX
#undef _HASH_JEN
#undef _get16bits
#undef _get16bits
#undef _HASH_SFH
#undef _HASH_FIND_IN_BKT
#undef _HASH_ADD_TO_BKT
#undef _HASH_DEL_IN_BKT
#undef _HASH_EXPAND_BUCKETS
#undef _HASH_SORT
#undef _HASH_SRT
#undef _HASH_SELECT
#undef _HASH_CLEAR
#undef _HASH_OVERHEAD
#undef _HASH_ITER
#undef _HASH_ITER
#undef _HASH_COUNT
#undef _HASH_CNT
#undef _HASH_SIGNATURE
#undef _HASH_BLOOM_SIGNATURE 
#endif /* _UTHASH_UNDEF */

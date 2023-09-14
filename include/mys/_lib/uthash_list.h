// https://troydhanson.github.io/uthash/
// Use #define _UTHASH_UNDEF_LIST to undef all these things

/************************************************/
/************************************************/
// [Part 1] Define UTLIST
/************************************************/
/************************************************/
#ifdef _UTHASH_DEFINE_LIST
#undef _UTHASH_DEFINE_LIST
#ifndef _UTLIST_H
#define _UTLIST_H

#define _UTLIST_VERSION 2.3.0

#include <assert.h>

/*
 * This file contains macros to manipulate singly and doubly-linked lists.
 *
 * 1. LL_ macros:  singly-linked lists.
 * 2. DL_ macros:  doubly-linked lists.
 * 3. CDL_ macros: circular doubly-linked lists.
 *
 * To use singly-linked lists, your structure must have a "next" pointer.
 * To use doubly-linked lists, your structure must "prev" and "next" pointers.
 * Either way, the pointer to the head of the list must be initialized to NULL.
 *
 * ----------------.EXAMPLE -------------------------
 * struct item {
 *      int id;
 *      struct item *prev, *next;
 * }
 *
 * struct item *list = NULL:
 *
 * int main() {
 *      struct item *item;
 *      ... allocate and populate item ...
 *      _DL_APPEND(list, item);
 * }
 * --------------------------------------------------
 *
 * For doubly-linked lists, the append and delete macros are O(1)
 * For singly-linked lists, append and delete are O(n) but prepend is O(1)
 * The sort macro is O(n log(n)) for all types of single/double/circular lists.
 */

/* These macros use decltype or the earlier __typeof GNU extension.
   As decltype is only available in newer compilers (VS2010 or gcc 4.3+
   when compiling c++ source) this code uses whatever method is needed
   or, for VS2008 where neither is available, uses casting workarounds. */
#if !defined(_LDECLTYPE) && !defined(_NO_DECLTYPE)
#if defined(_MSC_VER)   /* MS compiler */
#if _MSC_VER >= 1600 && defined(__cplusplus)  /* VS2010 or newer in C++ mode */
#define _LDECLTYPE(x) decltype(x)
#else                   /* VS2008 or older (or VS2010 in C mode) */
#define _NO_DECLTYPE
#endif
#elif defined(__MCST__)  /* Elbrus C Compiler */
#define _LDECLTYPE(x) __typeof(x)
#elif defined(__BORLANDC__) || defined(__ICCARM__) || defined(__LCC__) || defined(__WATCOMC__)
#define _NO_DECLTYPE
#else                   /* GNU, Sun and other compilers */
#define _LDECLTYPE(x) __typeof(x)
#endif
#endif

/* for VS2008 we use some workarounds to get around the lack of decltype,
 * namely, we always reassign our tmp variable to the list head if we need
 * to dereference its prev/next pointers, and save/restore the real head.*/
#ifdef _NO_DECLTYPE
#define _IF_NO_DECLTYPE(x) x
#define _LDECLTYPE(x) char*
#define _UTLIST_SV(elt,list) _tmp = (char*)(list); {char **_alias = (char**)&(list); *_alias = (elt); }
#define _UTLIST_NEXT(elt,list,next) ((char*)((list)->next))
#define _UTLIST_NEXTASGN(elt,list,to,next) { char **_alias = (char**)&((list)->next); *_alias=(char*)(to); }
/* #define _UTLIST_PREV(elt,list,prev) ((char*)((list)->prev)) */
#define _UTLIST_PREVASGN(elt,list,to,prev) { char **_alias = (char**)&((list)->prev); *_alias=(char*)(to); }
#define _UTLIST_RS(list) { char **_alias = (char**)&(list); *_alias=_tmp; }
#define _UTLIST_CASTASGN(a,b) { char **_alias = (char**)&(a); *_alias=(char*)(b); }
#else
#define _IF_NO_DECLTYPE(x)
#define _UTLIST_SV(elt,list)
#define _UTLIST_NEXT(elt,list,next) ((elt)->next)
#define _UTLIST_NEXTASGN(elt,list,to,next) ((elt)->next)=(to)
/* #define _UTLIST_PREV(elt,list,prev) ((elt)->prev) */
#define _UTLIST_PREVASGN(elt,list,to,prev) ((elt)->prev)=(to)
#define _UTLIST_RS(list)
#define _UTLIST_CASTASGN(a,b) (a)=(b)
#endif

/******************************************************************************
 * The sort macro is an adaptation of Simon Tatham's O(n log(n)) mergesort    *
 * Unwieldy variable names used here to avoid shadowing passed-in variables.  *
 *****************************************************************************/
#define _LL_SORT(list, cmp)                                                                     \
    _LL_SORT2(list, cmp, next)

#define _LL_SORT2(list, cmp, next)                                                              \
do {                                                                                           \
  _LDECLTYPE(list) _ls_p;                                                                       \
  _LDECLTYPE(list) _ls_q;                                                                       \
  _LDECLTYPE(list) _ls_e;                                                                       \
  _LDECLTYPE(list) _ls_tail;                                                                    \
  _IF_NO_DECLTYPE(_LDECLTYPE(list) _tmp;)                                                        \
  int _ls_insize, _ls_nmerges, _ls_psize, _ls_qsize, _ls_i, _ls_looping;                       \
  if (list) {                                                                                  \
    _ls_insize = 1;                                                                            \
    _ls_looping = 1;                                                                           \
    while (_ls_looping) {                                                                      \
      _UTLIST_CASTASGN(_ls_p,list);                                                             \
      (list) = NULL;                                                                           \
      _ls_tail = NULL;                                                                         \
      _ls_nmerges = 0;                                                                         \
      while (_ls_p) {                                                                          \
        _ls_nmerges++;                                                                         \
        _ls_q = _ls_p;                                                                         \
        _ls_psize = 0;                                                                         \
        for (_ls_i = 0; _ls_i < _ls_insize; _ls_i++) {                                         \
          _ls_psize++;                                                                         \
          _UTLIST_SV(_ls_q,list); _ls_q = _UTLIST_NEXT(_ls_q,list,next); _UTLIST_RS(list);        \
          if (!_ls_q) break;                                                                   \
        }                                                                                      \
        _ls_qsize = _ls_insize;                                                                \
        while (_ls_psize > 0 || (_ls_qsize > 0 && _ls_q)) {                                    \
          if (_ls_psize == 0) {                                                                \
            _ls_e = _ls_q; _UTLIST_SV(_ls_q,list); _ls_q =                                      \
              _UTLIST_NEXT(_ls_q,list,next); _UTLIST_RS(list); _ls_qsize--;                      \
          } else if (_ls_qsize == 0 || !_ls_q) {                                               \
            _ls_e = _ls_p; _UTLIST_SV(_ls_p,list); _ls_p =                                      \
              _UTLIST_NEXT(_ls_p,list,next); _UTLIST_RS(list); _ls_psize--;                      \
          } else if (cmp(_ls_p,_ls_q) <= 0) {                                                  \
            _ls_e = _ls_p; _UTLIST_SV(_ls_p,list); _ls_p =                                      \
              _UTLIST_NEXT(_ls_p,list,next); _UTLIST_RS(list); _ls_psize--;                      \
          } else {                                                                             \
            _ls_e = _ls_q; _UTLIST_SV(_ls_q,list); _ls_q =                                      \
              _UTLIST_NEXT(_ls_q,list,next); _UTLIST_RS(list); _ls_qsize--;                      \
          }                                                                                    \
          if (_ls_tail) {                                                                      \
            _UTLIST_SV(_ls_tail,list); _UTLIST_NEXTASGN(_ls_tail,list,_ls_e,next); _UTLIST_RS(list); \
          } else {                                                                             \
            _UTLIST_CASTASGN(list,_ls_e);                                                       \
          }                                                                                    \
          _ls_tail = _ls_e;                                                                    \
        }                                                                                      \
        _ls_p = _ls_q;                                                                         \
      }                                                                                        \
      if (_ls_tail) {                                                                          \
        _UTLIST_SV(_ls_tail,list); _UTLIST_NEXTASGN(_ls_tail,list,NULL,next); _UTLIST_RS(list);   \
      }                                                                                        \
      if (_ls_nmerges <= 1) {                                                                  \
        _ls_looping=0;                                                                         \
      }                                                                                        \
      _ls_insize *= 2;                                                                         \
    }                                                                                          \
  }                                                                                            \
} while (0)


#define _DL_SORT(list, cmp)                                                                     \
    _DL_SORT2(list, cmp, prev, next)

#define _DL_SORT2(list, cmp, prev, next)                                                        \
do {                                                                                           \
  _LDECLTYPE(list) _ls_p;                                                                       \
  _LDECLTYPE(list) _ls_q;                                                                       \
  _LDECLTYPE(list) _ls_e;                                                                       \
  _LDECLTYPE(list) _ls_tail;                                                                    \
  _IF_NO_DECLTYPE(_LDECLTYPE(list) _tmp;)                                                        \
  int _ls_insize, _ls_nmerges, _ls_psize, _ls_qsize, _ls_i, _ls_looping;                       \
  if (list) {                                                                                  \
    _ls_insize = 1;                                                                            \
    _ls_looping = 1;                                                                           \
    while (_ls_looping) {                                                                      \
      _UTLIST_CASTASGN(_ls_p,list);                                                             \
      (list) = NULL;                                                                           \
      _ls_tail = NULL;                                                                         \
      _ls_nmerges = 0;                                                                         \
      while (_ls_p) {                                                                          \
        _ls_nmerges++;                                                                         \
        _ls_q = _ls_p;                                                                         \
        _ls_psize = 0;                                                                         \
        for (_ls_i = 0; _ls_i < _ls_insize; _ls_i++) {                                         \
          _ls_psize++;                                                                         \
          _UTLIST_SV(_ls_q,list); _ls_q = _UTLIST_NEXT(_ls_q,list,next); _UTLIST_RS(list);        \
          if (!_ls_q) break;                                                                   \
        }                                                                                      \
        _ls_qsize = _ls_insize;                                                                \
        while ((_ls_psize > 0) || ((_ls_qsize > 0) && _ls_q)) {                                \
          if (_ls_psize == 0) {                                                                \
            _ls_e = _ls_q; _UTLIST_SV(_ls_q,list); _ls_q =                                      \
              _UTLIST_NEXT(_ls_q,list,next); _UTLIST_RS(list); _ls_qsize--;                      \
          } else if ((_ls_qsize == 0) || (!_ls_q)) {                                           \
            _ls_e = _ls_p; _UTLIST_SV(_ls_p,list); _ls_p =                                      \
              _UTLIST_NEXT(_ls_p,list,next); _UTLIST_RS(list); _ls_psize--;                      \
          } else if (cmp(_ls_p,_ls_q) <= 0) {                                                  \
            _ls_e = _ls_p; _UTLIST_SV(_ls_p,list); _ls_p =                                      \
              _UTLIST_NEXT(_ls_p,list,next); _UTLIST_RS(list); _ls_psize--;                      \
          } else {                                                                             \
            _ls_e = _ls_q; _UTLIST_SV(_ls_q,list); _ls_q =                                      \
              _UTLIST_NEXT(_ls_q,list,next); _UTLIST_RS(list); _ls_qsize--;                      \
          }                                                                                    \
          if (_ls_tail) {                                                                      \
            _UTLIST_SV(_ls_tail,list); _UTLIST_NEXTASGN(_ls_tail,list,_ls_e,next); _UTLIST_RS(list); \
          } else {                                                                             \
            _UTLIST_CASTASGN(list,_ls_e);                                                       \
          }                                                                                    \
          _UTLIST_SV(_ls_e,list); _UTLIST_PREVASGN(_ls_e,list,_ls_tail,prev); _UTLIST_RS(list);   \
          _ls_tail = _ls_e;                                                                    \
        }                                                                                      \
        _ls_p = _ls_q;                                                                         \
      }                                                                                        \
      _UTLIST_CASTASGN((list)->prev, _ls_tail);                                                 \
      _UTLIST_SV(_ls_tail,list); _UTLIST_NEXTASGN(_ls_tail,list,NULL,next); _UTLIST_RS(list);     \
      if (_ls_nmerges <= 1) {                                                                  \
        _ls_looping=0;                                                                         \
      }                                                                                        \
      _ls_insize *= 2;                                                                         \
    }                                                                                          \
  }                                                                                            \
} while (0)

#define _CDL_SORT(list, cmp)                                                                    \
    _CDL_SORT2(list, cmp, prev, next)

#define _CDL_SORT2(list, cmp, prev, next)                                                       \
do {                                                                                           \
  _LDECLTYPE(list) _ls_p;                                                                       \
  _LDECLTYPE(list) _ls_q;                                                                       \
  _LDECLTYPE(list) _ls_e;                                                                       \
  _LDECLTYPE(list) _ls_tail;                                                                    \
  _LDECLTYPE(list) _ls_oldhead;                                                                 \
  _LDECLTYPE(list) _tmp;                                                                        \
  int _ls_insize, _ls_nmerges, _ls_psize, _ls_qsize, _ls_i, _ls_looping;                       \
  if (list) {                                                                                  \
    _ls_insize = 1;                                                                            \
    _ls_looping = 1;                                                                           \
    while (_ls_looping) {                                                                      \
      _UTLIST_CASTASGN(_ls_p,list);                                                             \
      _UTLIST_CASTASGN(_ls_oldhead,list);                                                       \
      (list) = NULL;                                                                           \
      _ls_tail = NULL;                                                                         \
      _ls_nmerges = 0;                                                                         \
      while (_ls_p) {                                                                          \
        _ls_nmerges++;                                                                         \
        _ls_q = _ls_p;                                                                         \
        _ls_psize = 0;                                                                         \
        for (_ls_i = 0; _ls_i < _ls_insize; _ls_i++) {                                         \
          _ls_psize++;                                                                         \
          _UTLIST_SV(_ls_q,list);                                                               \
          if (_UTLIST_NEXT(_ls_q,list,next) == _ls_oldhead) {                                   \
            _ls_q = NULL;                                                                      \
          } else {                                                                             \
            _ls_q = _UTLIST_NEXT(_ls_q,list,next);                                              \
          }                                                                                    \
          _UTLIST_RS(list);                                                                     \
          if (!_ls_q) break;                                                                   \
        }                                                                                      \
        _ls_qsize = _ls_insize;                                                                \
        while (_ls_psize > 0 || (_ls_qsize > 0 && _ls_q)) {                                    \
          if (_ls_psize == 0) {                                                                \
            _ls_e = _ls_q; _UTLIST_SV(_ls_q,list); _ls_q =                                      \
              _UTLIST_NEXT(_ls_q,list,next); _UTLIST_RS(list); _ls_qsize--;                      \
            if (_ls_q == _ls_oldhead) { _ls_q = NULL; }                                        \
          } else if (_ls_qsize == 0 || !_ls_q) {                                               \
            _ls_e = _ls_p; _UTLIST_SV(_ls_p,list); _ls_p =                                      \
              _UTLIST_NEXT(_ls_p,list,next); _UTLIST_RS(list); _ls_psize--;                      \
            if (_ls_p == _ls_oldhead) { _ls_p = NULL; }                                        \
          } else if (cmp(_ls_p,_ls_q) <= 0) {                                                  \
            _ls_e = _ls_p; _UTLIST_SV(_ls_p,list); _ls_p =                                      \
              _UTLIST_NEXT(_ls_p,list,next); _UTLIST_RS(list); _ls_psize--;                      \
            if (_ls_p == _ls_oldhead) { _ls_p = NULL; }                                        \
          } else {                                                                             \
            _ls_e = _ls_q; _UTLIST_SV(_ls_q,list); _ls_q =                                      \
              _UTLIST_NEXT(_ls_q,list,next); _UTLIST_RS(list); _ls_qsize--;                      \
            if (_ls_q == _ls_oldhead) { _ls_q = NULL; }                                        \
          }                                                                                    \
          if (_ls_tail) {                                                                      \
            _UTLIST_SV(_ls_tail,list); _UTLIST_NEXTASGN(_ls_tail,list,_ls_e,next); _UTLIST_RS(list); \
          } else {                                                                             \
            _UTLIST_CASTASGN(list,_ls_e);                                                       \
          }                                                                                    \
          _UTLIST_SV(_ls_e,list); _UTLIST_PREVASGN(_ls_e,list,_ls_tail,prev); _UTLIST_RS(list);   \
          _ls_tail = _ls_e;                                                                    \
        }                                                                                      \
        _ls_p = _ls_q;                                                                         \
      }                                                                                        \
      _UTLIST_CASTASGN((list)->prev,_ls_tail);                                                  \
      _UTLIST_CASTASGN(_tmp,list);                                                              \
      _UTLIST_SV(_ls_tail,list); _UTLIST_NEXTASGN(_ls_tail,list,_tmp,next); _UTLIST_RS(list);     \
      if (_ls_nmerges <= 1) {                                                                  \
        _ls_looping=0;                                                                         \
      }                                                                                        \
      _ls_insize *= 2;                                                                         \
    }                                                                                          \
  }                                                                                            \
} while (0)

/******************************************************************************
 * singly linked list macros (non-circular)                                   *
 *****************************************************************************/
#define _LL_PREPEND(head,add)                                                                   \
    _LL_PREPEND2(head,add,next)

#define _LL_PREPEND2(head,add,next)                                                             \
do {                                                                                           \
  (add)->next = (head);                                                                        \
  (head) = (add);                                                                              \
} while (0)

#define _LL_CONCAT(head1,head2)                                                                 \
    _LL_CONCAT2(head1,head2,next)

#define _LL_CONCAT2(head1,head2,next)                                                           \
do {                                                                                           \
  _LDECLTYPE(head1) _tmp;                                                                       \
  if (head1) {                                                                                 \
    _tmp = (head1);                                                                            \
    while (_tmp->next) { _tmp = _tmp->next; }                                                  \
    _tmp->next=(head2);                                                                        \
  } else {                                                                                     \
    (head1)=(head2);                                                                           \
  }                                                                                            \
} while (0)

#define _LL_APPEND(head,add)                                                                    \
    _LL_APPEND2(head,add,next)

#define _LL_APPEND2(head,add,next)                                                              \
do {                                                                                           \
  _LDECLTYPE(head) _tmp;                                                                        \
  (add)->next=NULL;                                                                            \
  if (head) {                                                                                  \
    _tmp = (head);                                                                             \
    while (_tmp->next) { _tmp = _tmp->next; }                                                  \
    _tmp->next=(add);                                                                          \
  } else {                                                                                     \
    (head)=(add);                                                                              \
  }                                                                                            \
} while (0)

#define _LL_INSERT_INORDER(head,add,cmp)                                                        \
    _LL_INSERT_INORDER2(head,add,cmp,next)

#define _LL_INSERT_INORDER2(head,add,cmp,next)                                                  \
do {                                                                                           \
  _LDECLTYPE(head) _tmp;                                                                        \
  if (head) {                                                                                  \
    _LL_LOWER_BOUND2(head, _tmp, add, cmp, next);                                               \
    _LL_APPEND_ELEM2(head, _tmp, add, next);                                                    \
  } else {                                                                                     \
    (head) = (add);                                                                            \
    (head)->next = NULL;                                                                       \
  }                                                                                            \
} while (0)

#define _LL_LOWER_BOUND(head,elt,like,cmp)                                                      \
    _LL_LOWER_BOUND2(head,elt,like,cmp,next)

#define _LL_LOWER_BOUND2(head,elt,like,cmp,next)                                                \
  do {                                                                                         \
    if ((head) == NULL || (cmp(head, like)) >= 0) {                                            \
      (elt) = NULL;                                                                            \
    } else {                                                                                   \
      for ((elt) = (head); (elt)->next != NULL; (elt) = (elt)->next) {                         \
        if (cmp((elt)->next, like) >= 0) {                                                     \
          break;                                                                               \
        }                                                                                      \
      }                                                                                        \
    }                                                                                          \
  } while (0)

#define _LL_DELETE(head,del)                                                                    \
    _LL_DELETE2(head,del,next)

#define _LL_DELETE2(head,del,next)                                                              \
do {                                                                                           \
  _LDECLTYPE(head) _tmp;                                                                        \
  if ((head) == (del)) {                                                                       \
    (head)=(head)->next;                                                                       \
  } else {                                                                                     \
    _tmp = (head);                                                                             \
    while (_tmp->next && (_tmp->next != (del))) {                                              \
      _tmp = _tmp->next;                                                                       \
    }                                                                                          \
    if (_tmp->next) {                                                                          \
      _tmp->next = (del)->next;                                                                \
    }                                                                                          \
  }                                                                                            \
} while (0)

#define _LL_COUNT(head,el,counter)                                                              \
    _LL_COUNT2(head,el,counter,next)                                                            \

#define _LL_COUNT2(head,el,counter,next)                                                        \
do {                                                                                           \
  (counter) = 0;                                                                               \
  _LL_FOREACH2(head,el,next) { ++(counter); }                                                   \
} while (0)

#define _LL_FOREACH(head,el)                                                                    \
    _LL_FOREACH2(head,el,next)

#define _LL_FOREACH2(head,el,next)                                                              \
    for ((el) = (head); el; (el) = (el)->next)

#define _LL_FOREACH_SAFE(head,el,tmp)                                                           \
    _LL_FOREACH_SAFE2(head,el,tmp,next)

#define _LL_FOREACH_SAFE2(head,el,tmp,next)                                                     \
  for ((el) = (head); (el) && ((tmp) = (el)->next, 1); (el) = (tmp))

#define _LL_SEARCH_SCALAR(head,out,field,val)                                                   \
    _LL_SEARCH_SCALAR2(head,out,field,val,next)

#define _LL_SEARCH_SCALAR2(head,out,field,val,next)                                             \
do {                                                                                           \
    _LL_FOREACH2(head,out,next) {                                                               \
      if ((out)->field == (val)) break;                                                        \
    }                                                                                          \
} while (0)

#define _LL_SEARCH(head,out,elt,cmp)                                                            \
    _LL_SEARCH2(head,out,elt,cmp,next)

#define _LL_SEARCH2(head,out,elt,cmp,next)                                                      \
do {                                                                                           \
    _LL_FOREACH2(head,out,next) {                                                               \
      if ((cmp(out,elt))==0) break;                                                            \
    }                                                                                          \
} while (0)

#define _LL_REPLACE_ELEM2(head, el, add, next)                                                  \
do {                                                                                           \
 _LDECLTYPE(head) _tmp;                                                                         \
 assert((head) != NULL);                                                                       \
 assert((el) != NULL);                                                                         \
 assert((add) != NULL);                                                                        \
 (add)->next = (el)->next;                                                                     \
 if ((head) == (el)) {                                                                         \
  (head) = (add);                                                                              \
 } else {                                                                                      \
  _tmp = (head);                                                                               \
  while (_tmp->next && (_tmp->next != (el))) {                                                 \
   _tmp = _tmp->next;                                                                          \
  }                                                                                            \
  if (_tmp->next) {                                                                            \
    _tmp->next = (add);                                                                        \
  }                                                                                            \
 }                                                                                             \
} while (0)

#define _LL_REPLACE_ELEM(head, el, add)                                                         \
    _LL_REPLACE_ELEM2(head, el, add, next)

#define _LL_PREPEND_ELEM2(head, el, add, next)                                                  \
do {                                                                                           \
 if (el) {                                                                                     \
  _LDECLTYPE(head) _tmp;                                                                        \
  assert((head) != NULL);                                                                      \
  assert((add) != NULL);                                                                       \
  (add)->next = (el);                                                                          \
  if ((head) == (el)) {                                                                        \
   (head) = (add);                                                                             \
  } else {                                                                                     \
   _tmp = (head);                                                                              \
   while (_tmp->next && (_tmp->next != (el))) {                                                \
    _tmp = _tmp->next;                                                                         \
   }                                                                                           \
   if (_tmp->next) {                                                                           \
     _tmp->next = (add);                                                                       \
   }                                                                                           \
  }                                                                                            \
 } else {                                                                                      \
  _LL_APPEND2(head, add, next);                                                                 \
 }                                                                                             \
} while (0)                                                                                    \

#define _LL_PREPEND_ELEM(head, el, add)                                                         \
    _LL_PREPEND_ELEM2(head, el, add, next)

#define _LL_APPEND_ELEM2(head, el, add, next)                                                   \
do {                                                                                           \
 if (el) {                                                                                     \
  assert((head) != NULL);                                                                      \
  assert((add) != NULL);                                                                       \
  (add)->next = (el)->next;                                                                    \
  (el)->next = (add);                                                                          \
 } else {                                                                                      \
  _LL_PREPEND2(head, add, next);                                                                \
 }                                                                                             \
} while (0)                                                                                    \

#define _LL_APPEND_ELEM(head, el, add)                                                          \
    _LL_APPEND_ELEM2(head, el, add, next)

#ifdef _NO_DECLTYPE
/* Here are VS2008 / _NO_DECLTYPE replacements for a few functions */

#undef _LL_CONCAT2
#define _LL_CONCAT2(head1,head2,next)                                                           \
do {                                                                                           \
  char *_tmp;                                                                                  \
  if (head1) {                                                                                 \
    _tmp = (char*)(head1);                                                                     \
    while ((head1)->next) { (head1) = (head1)->next; }                                         \
    (head1)->next = (head2);                                                                   \
    _UTLIST_RS(head1);                                                                          \
  } else {                                                                                     \
    (head1)=(head2);                                                                           \
  }                                                                                            \
} while (0)

#undef _LL_APPEND2
#define _LL_APPEND2(head,add,next)                                                              \
do {                                                                                           \
  if (head) {                                                                                  \
    (add)->next = head;     /* use add->next as a temp variable */                             \
    while ((add)->next->next) { (add)->next = (add)->next->next; }                             \
    (add)->next->next=(add);                                                                   \
  } else {                                                                                     \
    (head)=(add);                                                                              \
  }                                                                                            \
  (add)->next=NULL;                                                                            \
} while (0)

#undef _LL_INSERT_INORDER2
#define _LL_INSERT_INORDER2(head,add,cmp,next)                                                  \
do {                                                                                           \
  if ((head) == NULL || (cmp(head, add)) >= 0) {                                               \
    (add)->next = (head);                                                                      \
    (head) = (add);                                                                            \
  } else {                                                                                     \
    char *_tmp = (char*)(head);                                                                \
    while ((head)->next != NULL && (cmp((head)->next, add)) < 0) {                             \
      (head) = (head)->next;                                                                   \
    }                                                                                          \
    (add)->next = (head)->next;                                                                \
    (head)->next = (add);                                                                      \
    _UTLIST_RS(head);                                                                           \
  }                                                                                            \
} while (0)

#undef _LL_DELETE2
#define _LL_DELETE2(head,del,next)                                                              \
do {                                                                                           \
  if ((head) == (del)) {                                                                       \
    (head)=(head)->next;                                                                       \
  } else {                                                                                     \
    char *_tmp = (char*)(head);                                                                \
    while ((head)->next && ((head)->next != (del))) {                                          \
      (head) = (head)->next;                                                                   \
    }                                                                                          \
    if ((head)->next) {                                                                        \
      (head)->next = ((del)->next);                                                            \
    }                                                                                          \
    _UTLIST_RS(head);                                                                           \
  }                                                                                            \
} while (0)

#undef _LL_REPLACE_ELEM2
#define _LL_REPLACE_ELEM2(head, el, add, next)                                                  \
do {                                                                                           \
  assert((head) != NULL);                                                                      \
  assert((el) != NULL);                                                                        \
  assert((add) != NULL);                                                                       \
  if ((head) == (el)) {                                                                        \
    (head) = (add);                                                                            \
  } else {                                                                                     \
    (add)->next = head;                                                                        \
    while ((add)->next->next && ((add)->next->next != (el))) {                                 \
      (add)->next = (add)->next->next;                                                         \
    }                                                                                          \
    if ((add)->next->next) {                                                                   \
      (add)->next->next = (add);                                                               \
    }                                                                                          \
  }                                                                                            \
  (add)->next = (el)->next;                                                                    \
} while (0)

#undef _LL_PREPEND_ELEM2
#define _LL_PREPEND_ELEM2(head, el, add, next)                                                  \
do {                                                                                           \
  if (el) {                                                                                    \
    assert((head) != NULL);                                                                    \
    assert((add) != NULL);                                                                     \
    if ((head) == (el)) {                                                                      \
      (head) = (add);                                                                          \
    } else {                                                                                   \
      (add)->next = (head);                                                                    \
      while ((add)->next->next && ((add)->next->next != (el))) {                               \
        (add)->next = (add)->next->next;                                                       \
      }                                                                                        \
      if ((add)->next->next) {                                                                 \
        (add)->next->next = (add);                                                             \
      }                                                                                        \
    }                                                                                          \
    (add)->next = (el);                                                                        \
  } else {                                                                                     \
    _LL_APPEND2(head, add, next);                                                               \
  }                                                                                            \
} while (0)                                                                                    \

#endif /* _NO_DECLTYPE */

/******************************************************************************
 * doubly linked list macros (non-circular)                                   *
 *****************************************************************************/
#define _DL_PREPEND(head,add)                                                                   \
    _DL_PREPEND2(head,add,prev,next)

#define _DL_PREPEND2(head,add,prev,next)                                                        \
do {                                                                                           \
 (add)->next = (head);                                                                         \
 if (head) {                                                                                   \
   (add)->prev = (head)->prev;                                                                 \
   (head)->prev = (add);                                                                       \
 } else {                                                                                      \
   (add)->prev = (add);                                                                        \
 }                                                                                             \
 (head) = (add);                                                                               \
} while (0)

#define _DL_APPEND(head,add)                                                                    \
    _DL_APPEND2(head,add,prev,next)

#define _DL_APPEND2(head,add,prev,next)                                                         \
do {                                                                                           \
  if (head) {                                                                                  \
      (add)->prev = (head)->prev;                                                              \
      (head)->prev->next = (add);                                                              \
      (head)->prev = (add);                                                                    \
      (add)->next = NULL;                                                                      \
  } else {                                                                                     \
      (head)=(add);                                                                            \
      (head)->prev = (head);                                                                   \
      (head)->next = NULL;                                                                     \
  }                                                                                            \
} while (0)

#define _DL_INSERT_INORDER(head,add,cmp)                                                        \
    _DL_INSERT_INORDER2(head,add,cmp,prev,next)

#define _DL_INSERT_INORDER2(head,add,cmp,prev,next)                                             \
do {                                                                                           \
  _LDECLTYPE(head) _tmp;                                                                        \
  if (head) {                                                                                  \
    _DL_LOWER_BOUND2(head, _tmp, add, cmp, next);                                               \
    _DL_APPEND_ELEM2(head, _tmp, add, prev, next);                                              \
  } else {                                                                                     \
    (head) = (add);                                                                            \
    (head)->prev = (head);                                                                     \
    (head)->next = NULL;                                                                       \
  }                                                                                            \
} while (0)

#define _DL_LOWER_BOUND(head,elt,like,cmp)                                                      \
    _DL_LOWER_BOUND2(head,elt,like,cmp,next)

#define _DL_LOWER_BOUND2(head,elt,like,cmp,next)                                                \
do {                                                                                           \
  if ((head) == NULL || (cmp(head, like)) >= 0) {                                              \
    (elt) = NULL;                                                                              \
  } else {                                                                                     \
    for ((elt) = (head); (elt)->next != NULL; (elt) = (elt)->next) {                           \
      if ((cmp((elt)->next, like)) >= 0) {                                                     \
        break;                                                                                 \
      }                                                                                        \
    }                                                                                          \
  }                                                                                            \
} while (0)

#define _DL_CONCAT(head1,head2)                                                                 \
    _DL_CONCAT2(head1,head2,prev,next)

#define _DL_CONCAT2(head1,head2,prev,next)                                                      \
do {                                                                                           \
  _LDECLTYPE(head1) _tmp;                                                                       \
  if (head2) {                                                                                 \
    if (head1) {                                                                               \
        _UTLIST_CASTASGN(_tmp, (head2)->prev);                                                  \
        (head2)->prev = (head1)->prev;                                                         \
        (head1)->prev->next = (head2);                                                         \
        _UTLIST_CASTASGN((head1)->prev, _tmp);                                                  \
    } else {                                                                                   \
        (head1)=(head2);                                                                       \
    }                                                                                          \
  }                                                                                            \
} while (0)

#define _DL_DELETE(head,del)                                                                    \
    _DL_DELETE2(head,del,prev,next)

#define _DL_DELETE2(head,del,prev,next)                                                         \
do {                                                                                           \
  assert((head) != NULL);                                                                      \
  assert((del)->prev != NULL);                                                                 \
  if ((del)->prev == (del)) {                                                                  \
      (head)=NULL;                                                                             \
  } else if ((del) == (head)) {                                                                \
      assert((del)->next != NULL);                                                             \
      (del)->next->prev = (del)->prev;                                                         \
      (head) = (del)->next;                                                                    \
  } else {                                                                                     \
      (del)->prev->next = (del)->next;                                                         \
      if ((del)->next) {                                                                       \
          (del)->next->prev = (del)->prev;                                                     \
      } else {                                                                                 \
          (head)->prev = (del)->prev;                                                          \
      }                                                                                        \
  }                                                                                            \
} while (0)

#define _DL_COUNT(head,el,counter)                                                              \
    _DL_COUNT2(head,el,counter,next)                                                            \

#define _DL_COUNT2(head,el,counter,next)                                                        \
do {                                                                                           \
  (counter) = 0;                                                                               \
  _DL_FOREACH2(head,el,next) { ++(counter); }                                                   \
} while (0)

#define _DL_FOREACH(head,el)                                                                    \
    _DL_FOREACH2(head,el,next)

#define _DL_FOREACH2(head,el,next)                                                              \
    for ((el) = (head); el; (el) = (el)->next)

/* this version is safe for deleting the elements during iteration */
#define _DL_FOREACH_SAFE(head,el,tmp)                                                           \
    _DL_FOREACH_SAFE2(head,el,tmp,next)

#define _DL_FOREACH_SAFE2(head,el,tmp,next)                                                     \
  for ((el) = (head); (el) && ((tmp) = (el)->next, 1); (el) = (tmp))

/* these are identical to their singly-linked list counterparts */
#define _DL_SEARCH_SCALAR _LL_SEARCH_SCALAR
#define _DL_SEARCH _LL_SEARCH
#define _DL_SEARCH_SCALAR2 _LL_SEARCH_SCALAR2
#define _DL_SEARCH2 _LL_SEARCH2

#define _DL_REPLACE_ELEM2(head, el, add, prev, next)                                            \
do {                                                                                           \
 assert((head) != NULL);                                                                       \
 assert((el) != NULL);                                                                         \
 assert((add) != NULL);                                                                        \
 if ((head) == (el)) {                                                                         \
  (head) = (add);                                                                              \
  (add)->next = (el)->next;                                                                    \
  if ((el)->next == NULL) {                                                                    \
   (add)->prev = (add);                                                                        \
  } else {                                                                                     \
   (add)->prev = (el)->prev;                                                                   \
   (add)->next->prev = (add);                                                                  \
  }                                                                                            \
 } else {                                                                                      \
  (add)->next = (el)->next;                                                                    \
  (add)->prev = (el)->prev;                                                                    \
  (add)->prev->next = (add);                                                                   \
  if ((el)->next == NULL) {                                                                    \
   (head)->prev = (add);                                                                       \
  } else {                                                                                     \
   (add)->next->prev = (add);                                                                  \
  }                                                                                            \
 }                                                                                             \
} while (0)

#define _DL_REPLACE_ELEM(head, el, add)                                                         \
    _DL_REPLACE_ELEM2(head, el, add, prev, next)

#define _DL_PREPEND_ELEM2(head, el, add, prev, next)                                            \
do {                                                                                           \
 if (el) {                                                                                     \
  assert((head) != NULL);                                                                      \
  assert((add) != NULL);                                                                       \
  (add)->next = (el);                                                                          \
  (add)->prev = (el)->prev;                                                                    \
  (el)->prev = (add);                                                                          \
  if ((head) == (el)) {                                                                        \
   (head) = (add);                                                                             \
  } else {                                                                                     \
   (add)->prev->next = (add);                                                                  \
  }                                                                                            \
 } else {                                                                                      \
  _DL_APPEND2(head, add, prev, next);                                                           \
 }                                                                                             \
} while (0)                                                                                    \

#define _DL_PREPEND_ELEM(head, el, add)                                                         \
    _DL_PREPEND_ELEM2(head, el, add, prev, next)

#define _DL_APPEND_ELEM2(head, el, add, prev, next)                                             \
do {                                                                                           \
 if (el) {                                                                                     \
  assert((head) != NULL);                                                                      \
  assert((add) != NULL);                                                                       \
  (add)->next = (el)->next;                                                                    \
  (add)->prev = (el);                                                                          \
  (el)->next = (add);                                                                          \
  if ((add)->next) {                                                                           \
   (add)->next->prev = (add);                                                                  \
  } else {                                                                                     \
   (head)->prev = (add);                                                                       \
  }                                                                                            \
 } else {                                                                                      \
  _DL_PREPEND2(head, add, prev, next);                                                          \
 }                                                                                             \
} while (0)                                                                                    \

#define _DL_APPEND_ELEM(head, el, add)                                                          \
   _DL_APPEND_ELEM2(head, el, add, prev, next)

#ifdef _NO_DECLTYPE
/* Here are VS2008 / _NO_DECLTYPE replacements for a few functions */

#undef _DL_INSERT_INORDER2
#define _DL_INSERT_INORDER2(head,add,cmp,prev,next)                                             \
do {                                                                                           \
  if ((head) == NULL) {                                                                        \
    (add)->prev = (add);                                                                       \
    (add)->next = NULL;                                                                        \
    (head) = (add);                                                                            \
  } else if ((cmp(head, add)) >= 0) {                                                          \
    (add)->prev = (head)->prev;                                                                \
    (add)->next = (head);                                                                      \
    (head)->prev = (add);                                                                      \
    (head) = (add);                                                                            \
  } else {                                                                                     \
    char *_tmp = (char*)(head);                                                                \
    while ((head)->next && (cmp((head)->next, add)) < 0) {                                     \
      (head) = (head)->next;                                                                   \
    }                                                                                          \
    (add)->prev = (head);                                                                      \
    (add)->next = (head)->next;                                                                \
    (head)->next = (add);                                                                      \
    _UTLIST_RS(head);                                                                           \
    if ((add)->next) {                                                                         \
      (add)->next->prev = (add);                                                               \
    } else {                                                                                   \
      (head)->prev = (add);                                                                    \
    }                                                                                          \
  }                                                                                            \
} while (0)
#endif /* _NO_DECLTYPE */

/******************************************************************************
 * circular doubly linked list macros                                         *
 *****************************************************************************/
#define _CDL_APPEND(head,add)                                                                   \
    _CDL_APPEND2(head,add,prev,next)

#define _CDL_APPEND2(head,add,prev,next)                                                        \
do {                                                                                           \
 if (head) {                                                                                   \
   (add)->prev = (head)->prev;                                                                 \
   (add)->next = (head);                                                                       \
   (head)->prev = (add);                                                                       \
   (add)->prev->next = (add);                                                                  \
 } else {                                                                                      \
   (add)->prev = (add);                                                                        \
   (add)->next = (add);                                                                        \
   (head) = (add);                                                                             \
 }                                                                                             \
} while (0)

#define _CDL_PREPEND(head,add)                                                                  \
    _CDL_PREPEND2(head,add,prev,next)

#define _CDL_PREPEND2(head,add,prev,next)                                                       \
do {                                                                                           \
 if (head) {                                                                                   \
   (add)->prev = (head)->prev;                                                                 \
   (add)->next = (head);                                                                       \
   (head)->prev = (add);                                                                       \
   (add)->prev->next = (add);                                                                  \
 } else {                                                                                      \
   (add)->prev = (add);                                                                        \
   (add)->next = (add);                                                                        \
 }                                                                                             \
 (head) = (add);                                                                               \
} while (0)

#define _CDL_INSERT_INORDER(head,add,cmp)                                                       \
    _CDL_INSERT_INORDER2(head,add,cmp,prev,next)

#define _CDL_INSERT_INORDER2(head,add,cmp,prev,next)                                            \
do {                                                                                           \
  _LDECLTYPE(head) _tmp;                                                                        \
  if (head) {                                                                                  \
    _CDL_LOWER_BOUND2(head, _tmp, add, cmp, next);                                              \
    _CDL_APPEND_ELEM2(head, _tmp, add, prev, next);                                             \
  } else {                                                                                     \
    (head) = (add);                                                                            \
    (head)->next = (head);                                                                     \
    (head)->prev = (head);                                                                     \
  }                                                                                            \
} while (0)

#define _CDL_LOWER_BOUND(head,elt,like,cmp)                                                     \
    _CDL_LOWER_BOUND2(head,elt,like,cmp,next)

#define _CDL_LOWER_BOUND2(head,elt,like,cmp,next)                                               \
do {                                                                                           \
  if ((head) == NULL || (cmp(head, like)) >= 0) {                                              \
    (elt) = NULL;                                                                              \
  } else {                                                                                     \
    for ((elt) = (head); (elt)->next != (head); (elt) = (elt)->next) {                         \
      if ((cmp((elt)->next, like)) >= 0) {                                                     \
        break;                                                                                 \
      }                                                                                        \
    }                                                                                          \
  }                                                                                            \
} while (0)

#define _CDL_DELETE(head,del)                                                                   \
    _CDL_DELETE2(head,del,prev,next)

#define _CDL_DELETE2(head,del,prev,next)                                                        \
do {                                                                                           \
  if (((head)==(del)) && ((head)->next == (head))) {                                           \
      (head) = NULL;                                                                           \
  } else {                                                                                     \
     (del)->next->prev = (del)->prev;                                                          \
     (del)->prev->next = (del)->next;                                                          \
     if ((del) == (head)) (head)=(del)->next;                                                  \
  }                                                                                            \
} while (0)

#define _CDL_COUNT(head,el,counter)                                                             \
    _CDL_COUNT2(head,el,counter,next)                                                           \

#define _CDL_COUNT2(head, el, counter,next)                                                     \
do {                                                                                           \
  (counter) = 0;                                                                               \
  _CDL_FOREACH2(head,el,next) { ++(counter); }                                                  \
} while (0)

#define _CDL_FOREACH(head,el)                                                                   \
    _CDL_FOREACH2(head,el,next)

#define _CDL_FOREACH2(head,el,next)                                                             \
    for ((el)=(head);el;(el)=(((el)->next==(head)) ? NULL : (el)->next))

#define _CDL_FOREACH_SAFE(head,el,tmp1,tmp2)                                                    \
    _CDL_FOREACH_SAFE2(head,el,tmp1,tmp2,prev,next)

#define _CDL_FOREACH_SAFE2(head,el,tmp1,tmp2,prev,next)                                         \
  for ((el) = (head), (tmp1) = (head) ? (head)->prev : NULL;                                   \
       (el) && ((tmp2) = (el)->next, 1);                                                       \
       (el) = ((el) == (tmp1) ? NULL : (tmp2)))

#define _CDL_SEARCH_SCALAR(head,out,field,val)                                                  \
    _CDL_SEARCH_SCALAR2(head,out,field,val,next)

#define _CDL_SEARCH_SCALAR2(head,out,field,val,next)                                            \
do {                                                                                           \
    _CDL_FOREACH2(head,out,next) {                                                              \
      if ((out)->field == (val)) break;                                                        \
    }                                                                                          \
} while (0)

#define _CDL_SEARCH(head,out,elt,cmp)                                                           \
    _CDL_SEARCH2(head,out,elt,cmp,next)

#define _CDL_SEARCH2(head,out,elt,cmp,next)                                                     \
do {                                                                                           \
    _CDL_FOREACH2(head,out,next) {                                                              \
      if ((cmp(out,elt))==0) break;                                                            \
    }                                                                                          \
} while (0)

#define _CDL_REPLACE_ELEM2(head, el, add, prev, next)                                           \
do {                                                                                           \
 assert((head) != NULL);                                                                       \
 assert((el) != NULL);                                                                         \
 assert((add) != NULL);                                                                        \
 if ((el)->next == (el)) {                                                                     \
  (add)->next = (add);                                                                         \
  (add)->prev = (add);                                                                         \
  (head) = (add);                                                                              \
 } else {                                                                                      \
  (add)->next = (el)->next;                                                                    \
  (add)->prev = (el)->prev;                                                                    \
  (add)->next->prev = (add);                                                                   \
  (add)->prev->next = (add);                                                                   \
  if ((head) == (el)) {                                                                        \
   (head) = (add);                                                                             \
  }                                                                                            \
 }                                                                                             \
} while (0)

#define _CDL_REPLACE_ELEM(head, el, add)                                                        \
    _CDL_REPLACE_ELEM2(head, el, add, prev, next)

#define _CDL_PREPEND_ELEM2(head, el, add, prev, next)                                           \
do {                                                                                           \
  if (el) {                                                                                    \
    assert((head) != NULL);                                                                    \
    assert((add) != NULL);                                                                     \
    (add)->next = (el);                                                                        \
    (add)->prev = (el)->prev;                                                                  \
    (el)->prev = (add);                                                                        \
    (add)->prev->next = (add);                                                                 \
    if ((head) == (el)) {                                                                      \
      (head) = (add);                                                                          \
    }                                                                                          \
  } else {                                                                                     \
    _CDL_APPEND2(head, add, prev, next);                                                        \
  }                                                                                            \
} while (0)

#define _CDL_PREPEND_ELEM(head, el, add)                                                        \
    _CDL_PREPEND_ELEM2(head, el, add, prev, next)

#define _CDL_APPEND_ELEM2(head, el, add, prev, next)                                            \
do {                                                                                           \
 if (el) {                                                                                     \
  assert((head) != NULL);                                                                      \
  assert((add) != NULL);                                                                       \
  (add)->next = (el)->next;                                                                    \
  (add)->prev = (el);                                                                          \
  (el)->next = (add);                                                                          \
  (add)->next->prev = (add);                                                                   \
 } else {                                                                                      \
  _CDL_PREPEND2(head, add, prev, next);                                                         \
 }                                                                                             \
} while (0)

#define _CDL_APPEND_ELEM(head, el, add)                                                         \
    _CDL_APPEND_ELEM2(head, el, add, prev, next)

#ifdef _NO_DECLTYPE
/* Here are VS2008 / _NO_DECLTYPE replacements for a few functions */

#undef _CDL_INSERT_INORDER2
#define _CDL_INSERT_INORDER2(head,add,cmp,prev,next)                                            \
do {                                                                                           \
  if ((head) == NULL) {                                                                        \
    (add)->prev = (add);                                                                       \
    (add)->next = (add);                                                                       \
    (head) = (add);                                                                            \
  } else if ((cmp(head, add)) >= 0) {                                                          \
    (add)->prev = (head)->prev;                                                                \
    (add)->next = (head);                                                                      \
    (add)->prev->next = (add);                                                                 \
    (head)->prev = (add);                                                                      \
    (head) = (add);                                                                            \
  } else {                                                                                     \
    char *_tmp = (char*)(head);                                                                \
    while ((char*)(head)->next != _tmp && (cmp((head)->next, add)) < 0) {                      \
      (head) = (head)->next;                                                                   \
    }                                                                                          \
    (add)->prev = (head);                                                                      \
    (add)->next = (head)->next;                                                                \
    (add)->next->prev = (add);                                                                 \
    (head)->next = (add);                                                                      \
    _UTLIST_RS(head);                                                                           \
  }                                                                                            \
} while (0)
#endif /* _NO_DECLTYPE */

#endif /* _UTLIST_H */
#endif /*_UTHASH_DEFINE_LIST*/

/************************************************/
/************************************************/
// [Part 2] Undef all these things if you want
/************************************************/
/************************************************/
#ifdef _UTHASH_UNDEF_LIST
#undef _UTHASH_UNDEF_LIST
#undef _UTLIST_H
#undef _UTLIST_VERSION
#undef _LDECLTYPE
#undef _NO_DECLTYPE
#undef _LDECLTYPE
#undef _NO_DECLTYPE
#undef _LDECLTYPE
#undef _IF_NO_DECLTYPE
#undef _LDECLTYPE
#undef _UTLIST_SV
#undef _UTLIST_NEXT
#undef _UTLIST_NEXTASGN
#undef _UTLIST_PREV
#undef _UTLIST_PREVASGN
#undef _UTLIST_RS
#undef _UTLIST_CASTASGN
#undef _IF_NO_DECLTYPE
#undef _UTLIST_SV
#undef _UTLIST_NEXT
#undef _UTLIST_NEXTASGN
#undef _UTLIST_PREV
#undef _UTLIST_PREVASGN
#undef _UTLIST_RS
#undef _UTLIST_CASTASGN
#undef _LL_SORT
#undef _LL_SORT2
#undef _DL_SORT
#undef _DL_SORT2
#undef _CDL_SORT
#undef _CDL_SORT2
#undef _LL_PREPEND
#undef _LL_PREPEND2
#undef _LL_CONCAT
#undef _LL_CONCAT2
#undef _LL_APPEND
#undef _LL_APPEND2
#undef _LL_INSERT_INORDER
#undef _LL_INSERT_INORDER2
#undef _LL_LOWER_BOUND
#undef _LL_LOWER_BOUND2
#undef _LL_DELETE
#undef _LL_DELETE2
#undef _LL_COUNT
#undef _LL_COUNT2
#undef _LL_FOREACH
#undef _LL_FOREACH2
#undef _LL_FOREACH_SAFE
#undef _LL_FOREACH_SAFE2
#undef _LL_SEARCH_SCALAR
#undef _LL_SEARCH_SCALAR2
#undef _LL_SEARCH
#undef _LL_SEARCH2
#undef _LL_REPLACE_ELEM2
#undef _LL_REPLACE_ELEM
#undef _LL_PREPEND_ELEM2
#undef _LL_PREPEND_ELEM
#undef _LL_APPEND_ELEM2
#undef _LL_APPEND_ELEM
#undef _LL_CONCAT2
#undef _LL_APPEND2
#undef _LL_INSERT_INORDER2
#undef _LL_DELETE2
#undef _LL_REPLACE_ELEM2
#undef _LL_PREPEND_ELEM2
#undef _DL_PREPEND
#undef _DL_PREPEND2
#undef _DL_APPEND
#undef _DL_APPEND2
#undef _DL_INSERT_INORDER
#undef _DL_INSERT_INORDER2
#undef _DL_LOWER_BOUND
#undef _DL_LOWER_BOUND2
#undef _DL_CONCAT
#undef _DL_CONCAT2
#undef _DL_DELETE
#undef _DL_DELETE2
#undef _DL_COUNT
#undef _DL_COUNT2
#undef _DL_FOREACH
#undef _DL_FOREACH2
#undef _DL_FOREACH_SAFE
#undef _DL_FOREACH_SAFE2
#undef _DL_SEARCH_SCALAR
#undef _DL_SEARCH
#undef _DL_SEARCH_SCALAR2
#undef _DL_SEARCH2
#undef _DL_REPLACE_ELEM2
#undef _DL_REPLACE_ELEM
#undef _DL_PREPEND_ELEM2
#undef _DL_PREPEND_ELEM
#undef _DL_APPEND_ELEM2
#undef _DL_APPEND_ELEM
#undef _DL_INSERT_INORDER2
#undef _CDL_APPEND
#undef _CDL_APPEND2
#undef _CDL_PREPEND
#undef _CDL_PREPEND2
#undef _CDL_INSERT_INORDER
#undef _CDL_INSERT_INORDER2
#undef _CDL_LOWER_BOUND
#undef _CDL_LOWER_BOUND2
#undef _CDL_DELETE
#undef _CDL_DELETE2
#undef _CDL_COUNT
#undef _CDL_COUNT2
#undef _CDL_FOREACH
#undef _CDL_FOREACH2
#undef _CDL_FOREACH_SAFE
#undef _CDL_FOREACH_SAFE2
#undef _CDL_SEARCH_SCALAR
#undef _CDL_SEARCH_SCALAR2
#undef _CDL_SEARCH
#undef _CDL_SEARCH2
#undef _CDL_REPLACE_ELEM2
#undef _CDL_REPLACE_ELEM
#undef _CDL_PREPEND_ELEM2
#undef _CDL_PREPEND_ELEM
#undef _CDL_APPEND_ELEM2
#undef _CDL_APPEND_ELEM
#undef _CDL_INSERT_INORDER2
#endif /* _UTHASH_UNDEF_LIST */

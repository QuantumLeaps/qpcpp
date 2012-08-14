/*  new.h

    Access to operator new() and newhandler()

    Copyright (c) 1990, 1992 by Borland International
    All Rights Reserved.
*/

#if !defined(__NEW_H)
#define __NEW_H

inline void *operator new(size_t, void *ptr) {
    return ptr;
}
//inline void *operator new[] (size_t, void *ptr) {
//    return ptr;
//}

#endif  /* __NEW_H */


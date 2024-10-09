#ifndef __LIB_USER_ASSERT_H
#define __LIB_USER_ASSERT_H
#include "basetype.h"

void user_spin(char *filename, const char *func, u64_t line, const char *condition);
#define panic(...) user_spin(__BASE_FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#ifdef NDEBUG
#define assert(CONDITION) ((void)0)
#else
#define assert(CONDITION)  \
    if (!(CONDITION))      \
    {                      \
        panic(#CONDITION); \
    }

#endif /*NDEBUG*/

#endif /*__LIB_USER_ASSERT_H*/
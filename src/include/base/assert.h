#ifndef __LIB_USER_ASSERT_H
#define __LIB_USER_ASSERT_H

void user_spin(str_t filename, cstr_t func, u64_t line, cstr_t condition);
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
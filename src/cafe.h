#pragma once

#include <setjmp.h>

/* Helper functions */

jmp_buf *cafe_hook_before_each();
jmp_buf *cafe_hook_after_each();
jmp_buf *cafe_hook_before();
jmp_buf *cafe_hook_after();

void cafe_return();
void cafe_save(jmp_buf *target);

void cafe_begin_suite(char *info);
void cafe_end_suite(char *info);

void cafe_begin_test(char *info);
void cafe_end_test(char *info);

void cafe_check(int err, char *message);

/* Helper macros */

#define CAFE_JMP(ID) cafe_jmp_##ID
#define CAFE_FLAG(ID) cafe_flag_##ID

#define CAFE_DO(ID, PRE, POST)                                                 \
    jmp_buf CAFE_JMP(ID);                                                      \
    int CAFE_FLAG(ID) = 0;                                                     \
    cafe_save(&CAFE_JMP(ID));                                                  \
    PRE if (setjmp(CAFE_JMP(ID))) {                                            \
        CAFE_FLAG(ID) = 0;                                                     \
        POST                                                                   \
    }                                                                          \
    else while (1) if (CAFE_FLAG(ID)) {                                        \
        longjmp(CAFE_JMP(ID), 1);                                              \
    }                                                                          \
    else while (CAFE_FLAG(ID)++ == 0)

#define CAFE_HOOK(ID, HOOK)                                                    \
    CAFE_DO(ID, if (setjmp(*HOOK()) == 0) {} else, cafe_return();)

/* User macros */

#define Assert(C) cafe_check(!(C), "Failed '" #C "'")
#define Fail(M) cafe_check(-1, M)

#define Describe(M) CAFE_DO(__LINE__, cafe_begin_suite(M);, cafe_end_suite(M);)

#define It(M) CAFE_DO(__LINE__, cafe_begin_test(M);, cafe_end_test(M);)

#define Before CAFE_HOOK(__LINE__, cafe_hook_before)
#define After CAFE_HOOK(__LINE__, cafe_hook_after)
#define BeforeEach CAFE_HOOK(__LINE__, cafe_hook_before_each)
#define AfterEach CAFE_HOOK(__LINE__, cafe_hook_after_each)

#define Cafe void cafe_main(int argc, char **argv)

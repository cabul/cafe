#pragma once

#include <setjmp.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#ifndef CAFE_MAX_LEVELS
#define CAFE_MAX_LEVELS 4
#endif

enum cafe_status {
    CAFE_FAILING = -1,
    CAFE_PASSING = 0,
    CAFE_PENDING = 1,
};

static int cafe_status;
static int cafe_level = 0;

static int cafe_passing = 0;
static int cafe_failing = 0;
static int cafe_pending = 0;

static jmp_buf cafe_be_hooks[CAFE_MAX_LEVELS];
static int cafe_be_set;
static jmp_buf cafe_ae_hooks[CAFE_MAX_LEVELS];
static int cafe_ae_set;

static int cafe_b_done;
static jmp_buf cafe_b_hooks[CAFE_MAX_LEVELS];
static int cafe_b_set;
static jmp_buf cafe_a_hooks[CAFE_MAX_LEVELS];
static int cafe_a_set;

static char *cafe_failure;
static jmp_buf cafe_return;
static int cafe_colors;

#define CAFE_SYM(COL, SYM) (cafe_colors ? COL SYM "\033[0m" : SYM)

#ifdef CAFE_ASCII
#define CAFE_SYM_PASSING CAFE_SYM("\033[1;32m", "+")
#define CAFE_SYM_PENDING CAFE_SYM("\033[1;36m", "o")
#define CAFE_SYM_FAILING CAFE_SYM("\033[1;31m", "-")
#else
#define CAFE_SYM_PASSING CAFE_SYM("\033[1;32m", "✔")
#define CAFE_SYM_PENDING CAFE_SYM("\033[1;36m", "∙")
#define CAFE_SYM_FAILING CAFE_SYM("\033[1;31m", "✘")
#endif

#define CAFE_GOTO(HOOK)                                                        \
    if (setjmp(cafe_return) == 0) {                                            \
        longjmp(HOOK, 1);                                                      \
    }

#define CAFE_JMP(CNT) cafe_jmp_##CNT
#define CAFE_FLAG(CNT) cafe_flag_##CNT

#define CAFE_DO(CNT, PRE, POST)                                                \
    jmp_buf CAFE_JMP(CNT);                                                     \
    int CAFE_FLAG(CNT) = 0;                                                    \
    PRE if (setjmp(CAFE_JMP(CNT))) {                                           \
        CAFE_FLAG(CNT) = 0;                                                    \
        POST                                                                   \
    }                                                                          \
    else while (1) if (CAFE_FLAG(CNT)) {                                       \
        longjmp(CAFE_JMP(CNT), 1);                                             \
    }                                                                          \
    else while (CAFE_FLAG(CNT)++ == 0)

#define CAFE_HOOK(CNT, HOOK)                                                   \
    CAFE_DO(CNT, cafe_##HOOK##_set |= (1 << cafe_level);                       \
            if (setjmp(cafe_##HOOK##_hooks[cafe_level]) == 0) {} else,         \
            longjmp(cafe_return, 1);)

#define CAFE_PRINT(...)                                                        \
    do {                                                                       \
        printf("%*s", cafe_level * 2 + 2, "");                                 \
        printf(__VA_ARGS__);                                                   \
    } while (0)

#define Assert(cond)                                                           \
    if (cond) {                                                                \
        cafe_status = CAFE_PASSING;                                            \
    } else {                                                                   \
        cafe_status = CAFE_FAILING;                                            \
        cafe_failure = "Failed '" #cond "'";                                   \
        break;                                                                 \
    }

#define Fail(message)                                                          \
    cafe_status = CAFE_FAILING;                                                \
    cafe_failure = message;                                                    \
    break;

#define Describe(message)                                                      \
    CAFE_DO(__LINE__, CAFE_PRINT("%s\n", message); ++cafe_level;               \
            cafe_b_done &= ~(1 << cafe_level);                                 \
            , if (cafe_b_done & cafe_a_set & (1 << cafe_level)) {              \
                CAFE_GOTO(cafe_a_hooks[cafe_level])                            \
            } --cafe_level;)

#define It(message)                                                            \
    CAFE_DO(__LINE__,                                                          \
            for (int i = 0; i <= cafe_level; ++i) {                            \
                if ((cafe_b_done & (1 << i)) == 0) {                           \
                    if (cafe_b_set & (1 << i)) {                               \
                        CAFE_GOTO(cafe_b_hooks[i])                             \
                    }                                                          \
                }                                                              \
                cafe_b_done |= (1 << i);                                       \
            } for (int i = 0; i <= cafe_level; ++i) {                          \
                if (cafe_be_set & (1 << i)) {                                  \
                    CAFE_GOTO(cafe_be_hooks[i])                                \
                }                                                              \
            } cafe_status = CAFE_PENDING;                                      \
            ,                                                                  \
            switch (cafe_status) {                                             \
                case CAFE_PASSING:                                             \
                    ++cafe_passing;                                            \
                    CAFE_PRINT("%s %s\n", CAFE_SYM_PASSING, message);          \
                    break;                                                     \
                case CAFE_PENDING:                                             \
                    ++cafe_pending;                                            \
                    CAFE_PRINT("%s %s\n", CAFE_SYM_PENDING, message);          \
                    break;                                                     \
                default:                                                       \
                    ++cafe_failing;                                            \
                    CAFE_PRINT("%s %s\n", CAFE_SYM_FAILING, message);          \
                    CAFE_PRINT("  » %s\n", cafe_failure);                      \
                    CAFE_PRINT("  » in %s at line %d\n", __FILE__, __LINE__);  \
            } for (int i = cafe_level; i >= 0; --i) {                          \
                if (cafe_ae_set & (1 << i)) {                                  \
                    CAFE_GOTO(cafe_ae_hooks[i])                                \
                }                                                              \
            })

#define BeforeEach CAFE_HOOK(__LINE__, be)
#define AfterEach CAFE_HOOK(__LINE__, ae)
#define Before CAFE_HOOK(__LINE__, b)
#define After CAFE_HOOK(__LINE__, a)

#define Cafe void cafe_main(int argc, char **argv)

static double cafe_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1e3 + tv.tv_usec * 1e-3;
}

void cafe_main(int argc, char **argv);

int main(int argc, char **argv) {
    cafe_colors = isatty(fileno(stdout));
    printf("\n");
    double cafe_dtime = cafe_time_ms();
    cafe_main(argc, argv);
    printf("\n");
    cafe_dtime = cafe_time_ms() - cafe_dtime;
    CAFE_PRINT("Results after %d tests (%.0f ms)\n",
               cafe_passing + cafe_pending + cafe_failing, cafe_dtime);
    if (cafe_passing) {
        CAFE_PRINT("%s %d passing\n", CAFE_SYM_PASSING, cafe_passing);
    }
    if (cafe_pending) {
        CAFE_PRINT("%s %d pending\n", CAFE_SYM_PENDING, cafe_pending);
    }
    if (cafe_failing) {
        CAFE_PRINT("%s %d failing\n", CAFE_SYM_FAILING, cafe_failing);
    }
    printf("\n");
    return cafe_failing;
}

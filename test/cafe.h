#pragma once

#include <setjmp.h>
#include <stdio.h>
#include <sys/time.h>

#ifndef CAFE_MAX_HOOKS
#define CAFE_MAX_HOOKS 16
#endif

#ifndef CAFE_MAX_LEVELS
#define CAFE_MAX_LEVELS 4
#endif

enum cafe_status {
    CAFE_ERROR = -1,
    CAFE_PASSING = 0,
    CAFE_PENDING = 1,
    CAFE_FAILING = 2,
};

static int cafe_status;
static int cafe_level = 0;
static int cafe_spaces = 2;

static int cafe_passing = 0;
static int cafe_failing = 0;
static int cafe_pending = 0;

static jmp_buf cafe_be_hooks[CAFE_MAX_HOOKS];
static jmp_buf cafe_ae_hooks[CAFE_MAX_HOOKS];

static int cafe_be_count[CAFE_MAX_LEVELS];
static int cafe_ae_count[CAFE_MAX_LEVELS];

static jmp_buf cafe_b_hooks[CAFE_MAX_LEVELS][CAFE_MAX_HOOKS];
static jmp_buf cafe_a_hooks[CAFE_MAX_LEVELS][CAFE_MAX_HOOKS];

static int cafe_b_count[CAFE_MAX_LEVELS];
static int cafe_b_done[CAFE_MAX_LEVELS];
static int cafe_a_count[CAFE_MAX_LEVELS];

static char *cafe_helper;
static jmp_buf cafe_return;

void cafe_main(int argc, char **argv);

#define CAFE_GOTO(hook)                                                        \
    if (setjmp(cafe_return) == 0) {                                            \
        longjmp(hook, 1);                                                      \
    }

#define CAFE_JMP(cnt) cafe_jmp_##cnt
#define CAFE_FLAG(cnt) cafe_flag_##cnt

#define CAFE_DO(cnt, pre, post)                                                \
    jmp_buf CAFE_JMP(cnt);                                                     \
    int CAFE_FLAG(cnt) = 0;                                                    \
    pre if (setjmp(CAFE_JMP(cnt))) {                                           \
        CAFE_FLAG(cnt) = 0;                                                    \
        post                                                                   \
    }                                                                          \
    else while (1) if (CAFE_FLAG(cnt)) {                                       \
        longjmp(CAFE_JMP(cnt), 1);                                             \
    }                                                                          \
    else while (CAFE_FLAG(cnt)++ == 0)

#define CAFE_RUN(cnt, fun, ...)                                                \
    CAFE_DO(cnt, cafe_begin_##fun(__VA_ARGS__);, cafe_end_##fun(__VA_ARGS__);)

#define CAFE_HOOK(cnt, hook)                                                   \
    CAFE_DO(cnt, if (setjmp(hook) == 0) {} else, longjmp(cafe_return, 1);)

#define CAFE_PRINT(...)                                                        \
    do {                                                                       \
        printf("%*s", cafe_level *cafe_spaces + cafe_spaces, "");              \
        printf(__VA_ARGS__);                                                   \
    } while (0)

#define Assert(cond)                                                           \
    if (cond) {                                                                \
        cafe_status = CAFE_PASSING;                                            \
    } else {                                                                   \
        cafe_status = CAFE_FAILING;                                            \
        cafe_helper = #cond;                                                   \
        break;                                                                 \
    }

#define Error(message)                                                         \
    cafe_status = CAFE_ERROR;                                                  \
    cafe_helper = message;                                                     \
    break;

#define Describe(message) CAFE_RUN(__LINE__, describe, message)

#define It(message) CAFE_RUN(__LINE__, it, message, __FILE__, __LINE__)

#define BeforeEach                                                             \
    CAFE_HOOK(__LINE__, cafe_be_hooks[cafe_be_count[cafe_level]++])

#define AfterEach                                                              \
    CAFE_HOOK(__LINE__, cafe_ae_hooks[cafe_ae_count[cafe_level]++])

#define Before                                                                 \
    CAFE_HOOK(__LINE__, cafe_b_hooks[cafe_level][cafe_b_count[cafe_level]++])

#define After                                                                  \
    CAFE_HOOK(__LINE__, cafe_a_hooks[cafe_level][cafe_a_count[cafe_level]++])

#define Cafe void cafe_main(int argc, char **argv)

static double cafe_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1e3 + tv.tv_usec * 1e-3;
}

static void cafe_begin_describe(char *message) {
    CAFE_PRINT("%s\n", message);
    cafe_be_count[cafe_level + 1] = cafe_be_count[cafe_level];
    cafe_ae_count[cafe_level + 1] = cafe_ae_count[cafe_level];
    ++cafe_level;
    cafe_b_done[cafe_level] = 0;
}

static void cafe_end_describe(char *message) {
    if (cafe_b_done[cafe_level]) {
        for (int i = cafe_a_count[cafe_level] - 1; i >= 0; --i) {
            CAFE_GOTO(cafe_a_hooks[cafe_level][i])
        }
    }
    --cafe_level;
}

static void cafe_begin_it(char *message, char *file, int line) {
    for (int i = 0; i <= cafe_level; ++i) {
        if (cafe_b_done[i] == 0) {
            for (int j = 0; j < cafe_b_count[i]; ++j) {
                CAFE_GOTO(cafe_b_hooks[i][j])
            }
            cafe_b_done[i] = 1;
        }
    }
    for (int i = 0; i < cafe_be_count[cafe_level]; ++i) {
        CAFE_GOTO(cafe_be_hooks[i])
    }
    cafe_status = CAFE_PENDING;
}

static void cafe_end_it(char *message, char *file, int line) {
    switch (cafe_status) {
    case CAFE_PASSING:
        ++cafe_passing;
        CAFE_PRINT("✓ %s\n", message);
        break;
    case CAFE_PENDING:
        CAFE_PRINT("• %s\n", message);
        ++cafe_pending;
        break;
    default:
        CAFE_PRINT("✗ %s\n", message);
        ++cafe_failing;
        ++cafe_level;
        if (cafe_status == CAFE_FAILING) {
            CAFE_PRINT("» Assertion '%s' failed\n", cafe_helper);
        } else {
            CAFE_PRINT("» %s\n", cafe_helper);
        }
        CAFE_PRINT("» in %s at line %d\n", file, line);
        --cafe_level;
    }
    for (int cafe_i = cafe_ae_count[cafe_level] - 1; cafe_i >= 0; --cafe_i) {
        CAFE_GOTO(cafe_ae_hooks[cafe_i])
    }
}

int main(int argc, char **argv) {
    printf("\n");
    double cafe_dtime = cafe_time_ms();
    cafe_main(argc, argv);
    printf("\n");
    cafe_dtime = cafe_time_ms() - cafe_dtime;
    CAFE_PRINT("Results after %d tests (%.0fms)\n",
               cafe_passing + cafe_pending + cafe_failing, cafe_dtime);
    if (cafe_passing) {
        CAFE_PRINT("✓ %d passing\n", cafe_passing);
    }
    if (cafe_pending) {
        CAFE_PRINT("• %d pending\n", cafe_pending);
    }
    if (cafe_failing) {
        CAFE_PRINT("✗ %d failing\n", cafe_failing);
    }
    printf("\n");
    return cafe_failing;
}

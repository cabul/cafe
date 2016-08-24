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

static int cafe_status = 0;
static int cafe_level = 0;
static int cafe_spaces = 2;

static int cafe_passing = 0;
static int cafe_failing = 0;
static int cafe_pending = 0;

static double cafe_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1e3 + tv.tv_usec * 1e-3;
}

static jmp_buf cafe_be_hooks[CAFE_MAX_HOOKS];
static jmp_buf cafe_ae_hooks[CAFE_MAX_HOOKS];

static int cafe_be_num_hooks[CAFE_MAX_LEVELS];
static int cafe_ae_num_hooks[CAFE_MAX_LEVELS];

static jmp_buf cafe_b_hooks[CAFE_MAX_LEVELS][CAFE_MAX_HOOKS];
static jmp_buf cafe_a_hooks[CAFE_MAX_LEVELS][CAFE_MAX_HOOKS];

static int cafe_b_num_hooks[CAFE_MAX_LEVELS];
static int cafe_b_hooks_done[CAFE_MAX_LEVELS];
static int cafe_a_num_hooks[CAFE_MAX_LEVELS];

static char *cafe_helper;
static jmp_buf cafe_return;

void cafe_main(int argc, char **argv);

#define cafe_counter_impl(idx) cafe_counter_##idx
#define cafe_counter(idx) cafe_counter_impl(idx)
#define cafe_loop_impl(idx) cafe_loop_##idx
#define cafe_loop(idx) cafe_loop_impl(idx)

#define cafe_print(...)                                                        \
    do {                                                                       \
        printf("%*s", cafe_level *cafe_spaces + cafe_spaces, "");              \
        printf(__VA_ARGS__);                                                   \
    } while (0)

#define Assert(cond)                                                           \
    if (!(cond)) {                                                             \
        cafe_status = 1;                                                       \
        cafe_helper = #cond;                                                   \
        break;                                                                 \
    }

#define Error(message)                                                         \
    cafe_status = -1;                                                          \
    cafe_helper = message;                                                     \
    break;

#define Describe(message)                                                      \
    cafe_print(message "\n");                                                  \
    cafe_be_num_hooks[cafe_level + 1] = cafe_be_num_hooks[cafe_level];         \
    cafe_ae_num_hooks[cafe_level + 1] = cafe_ae_num_hooks[cafe_level];         \
    ++cafe_level;                                                              \
    cafe_b_hooks_done[cafe_level] = 0;                                         \
    jmp_buf cafe_loop(__LINE__);                                               \
    int cafe_counter(__LINE__) = 0;                                            \
    if (setjmp(cafe_loop(__LINE__)) != 0) {                                    \
        if (cafe_b_hooks_done[cafe_level]) {                                   \
            for (int cafe_i = cafe_a_num_hooks[cafe_level] - 1; cafe_i >= 0;   \
                 --cafe_i) {                                                   \
                if (setjmp(cafe_return) == 0) {                                \
                    longjmp(cafe_a_hooks[cafe_level][cafe_i], 1);              \
                }                                                              \
            }                                                                  \
        }                                                                      \
        --cafe_level;                                                          \
    } else                                                                     \
        while (1)                                                              \
            if (cafe_counter(__LINE__)++) {                                    \
                longjmp(cafe_loop(__LINE__), 1);                               \
            } else

#define It(message)                                                            \
    for (int cafe_i = 0; cafe_i <= cafe_level; ++cafe_i) {                     \
        if (cafe_b_hooks_done[cafe_i] == 0) {                                  \
            for (int cafe_j = 0; cafe_j < cafe_b_num_hooks[cafe_i];            \
                 ++cafe_j) {                                                   \
                if (setjmp(cafe_return) == 0) {                                \
                    longjmp(cafe_b_hooks[cafe_i][cafe_j], 1);                  \
                }                                                              \
            }                                                                  \
            cafe_b_hooks_done[cafe_i] = 1;                                     \
        }                                                                      \
    }                                                                          \
    for (int cafe_i = 0; cafe_i < cafe_be_num_hooks[cafe_level]; ++cafe_i) {   \
        if (setjmp(cafe_return) == 0) {                                        \
            longjmp(cafe_be_hooks[cafe_i], 1);                                 \
        }                                                                      \
    }                                                                          \
    cafe_status = 0;                                                           \
    jmp_buf cafe_loop(__LINE__);                                               \
    int cafe_counter(__LINE__) = 0;                                            \
    if (setjmp(cafe_loop(__LINE__)) != 0) {                                    \
        if (cafe_status != 0) {                                                \
            cafe_print("✗ " message "\n");                                     \
            ++cafe_failing;                                                    \
            ++cafe_level;                                                      \
            if (cafe_status > 0) {                                             \
                cafe_print("» Assertion '%s' failed\n", cafe_helper);          \
            } else {                                                           \
                cafe_print("» %s\n", cafe_helper);                             \
            }                                                                  \
            cafe_print("» in %s at line %d\n", __FILE__, __LINE__);            \
            --cafe_level;                                                      \
        } else {                                                               \
            cafe_print("✓ " message "\n");                                     \
            ++cafe_passing;                                                    \
        }                                                                      \
        for (int cafe_i = cafe_ae_num_hooks[cafe_level] - 1; cafe_i >= 0;      \
             --cafe_i) {                                                       \
            if (setjmp(cafe_return) == 0) {                                    \
                longjmp(cafe_ae_hooks[cafe_i], 1);                             \
            }                                                                  \
        }                                                                      \
    } else                                                                     \
        while (1)                                                              \
            if (cafe_counter(__LINE__)) {                                      \
                longjmp(cafe_loop(__LINE__), 1);                               \
            } else                                                             \
                while (cafe_counter(__LINE__)++ == 0)

#define Pending(message)                                                       \
    cafe_print("• " message "\n");                                             \
    ++cafe_pending;

#define BeforeEach                                                             \
    jmp_buf cafe_loop(__LINE__);                                               \
    int cafe_counter(__LINE__) = 0;                                            \
    if (setjmp(cafe_be_hooks[cafe_be_num_hooks[cafe_level]++]) == 0) {         \
    } else if (setjmp(cafe_loop(__LINE__)) != 0) {                             \
        cafe_counter(__LINE__) = 0;                                            \
        longjmp(cafe_return, 1);                                               \
    } else                                                                     \
        while (1)                                                              \
            if (cafe_counter(__LINE__)++) {                                    \
                longjmp(cafe_loop(__LINE__), 1);                               \
            } else

#define AfterEach                                                              \
    jmp_buf cafe_loop(__LINE__);                                               \
    int cafe_counter(__LINE__) = 0;                                            \
    if (setjmp(cafe_ae_hooks[cafe_ae_num_hooks[cafe_level]++]) == 0) {         \
    } else if (setjmp(cafe_loop(__LINE__)) != 0) {                             \
        cafe_counter(__LINE__) = 0;                                            \
        longjmp(cafe_return, 1);                                               \
    } else                                                                     \
        while (1)                                                              \
            if (cafe_counter(__LINE__)++) {                                    \
                longjmp(cafe_loop(__LINE__), 1);                               \
            } else

#define Before                                                                 \
    jmp_buf cafe_loop(__LINE__);                                               \
    int cafe_counter(__LINE__) = 0;                                            \
    if (setjmp(cafe_b_hooks[cafe_level][cafe_b_num_hooks[cafe_level]++]) ==    \
        0) {                                                                   \
    } else if (setjmp(cafe_loop(__LINE__)) != 0) {                             \
        cafe_counter(__LINE__) = 0;                                            \
        longjmp(cafe_return, 1);                                               \
    } else                                                                     \
        while (1)                                                              \
            if (cafe_counter(__LINE__)++) {                                    \
                longjmp(cafe_loop(__LINE__), 1);                               \
            } else

#define After                                                                  \
    jmp_buf cafe_loop(__LINE__);                                               \
    int cafe_counter(__LINE__) = 0;                                            \
    if (setjmp(cafe_a_hooks[cafe_level][cafe_a_num_hooks[cafe_level]++]) ==    \
        0) {                                                                   \
    } else if (setjmp(cafe_loop(__LINE__)) != 0) {                             \
        cafe_counter(__LINE__) = 0;                                            \
        longjmp(cafe_return, 1);                                               \
    } else                                                                     \
        while (1)                                                              \
            if (cafe_counter(__LINE__)++) {                                    \
                longjmp(cafe_loop(__LINE__), 1);                               \
            } else

#define Cafe                                                                   \
    int main(int argc, char **argv) {                                          \
        printf("\n");                                                          \
        double cafe_dtime = cafe_time_ms();                                    \
        cafe_main(argc, argv);                                                 \
        printf("\n");                                                          \
        cafe_dtime = cafe_time_ms() - cafe_dtime;                              \
        cafe_print("Results after %d tests (%.0fms)\n",                        \
                   cafe_passing + cafe_pending + cafe_failing, cafe_dtime);    \
        if (cafe_passing) {                                                    \
            cafe_print("✓ %d passing\n", cafe_passing);                        \
        }                                                                      \
        if (cafe_pending) {                                                    \
            cafe_print("• %d pending\n", cafe_pending);                        \
        }                                                                      \
        if (cafe_failing) {                                                    \
            cafe_print("✗ %d failing\n", cafe_failing);                        \
        }                                                                      \
        printf("\n");                                                          \
        return cafe_failing;                                                   \
    }                                                                          \
                                                                               \
    void cafe_main(int argc, char **argv)

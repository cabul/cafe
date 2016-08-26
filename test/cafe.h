#pragma once

#include <setjmp.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

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

static jmp_buf cafe_return;
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

#define cafe_print(...)                                                        \
    do {                                                                       \
        printf("%*s", cafe_level *cafe_spaces + cafe_spaces, "");              \
        printf(__VA_ARGS__);                                                   \
    } while (0)

#ifdef CAFE_COLORS
#define cafe_term(esc)                                                         \
    do {                                                                       \
        if (isatty(fileno(stdout))) {                                          \
            printf(esc);                                                       \
        }                                                                      \
    } while (0)
#else
#define cafe_term(esc)
#endif

#define Assert(cond)                                                           \
    if (!(cond)) {                                                             \
        cafe_status = -1;                                                      \
        cafe_helper = #cond;                                                   \
        break;                                                                 \
    }

#define Describe(message, block)                                               \
    ;                                                                          \
    cafe_print(message "\n");                                                  \
    cafe_be_num_hooks[cafe_level + 1] = cafe_be_num_hooks[cafe_level];         \
    cafe_ae_num_hooks[cafe_level + 1] = cafe_ae_num_hooks[cafe_level];         \
    ++cafe_level;                                                              \
    cafe_b_hooks_done[cafe_level] = 0;                                         \
    do {                                                                       \
        block                                                                  \
    } while (0);                                                               \
    if (cafe_b_hooks_done[cafe_level]) {                                       \
        for (int cafe_i = cafe_a_num_hooks[cafe_level] - 1; cafe_i >= 0;       \
             --cafe_i) {                                                       \
            if (setjmp(cafe_return) == 0) {                                    \
                longjmp(cafe_a_hooks[cafe_level][cafe_i], 1);                  \
            }                                                                  \
        }                                                                      \
    }                                                                          \
    --cafe_level;

#define It(message, block)                                                     \
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
    do {                                                                       \
        block                                                                  \
    } while (0);                                                               \
    if (cafe_status != 0) {                                                    \
        cafe_term("\033[1;31m");                                               \
        cafe_print("✗ " message "\n");                                         \
        cafe_term("\033[0m");                                                  \
        ++cafe_failing;                                                        \
        ++cafe_level;                                                          \
        cafe_print("Assertion '%s' failed\n", cafe_helper);                    \
        --cafe_level;                                                          \
    } else {                                                                   \
        cafe_term("\033[1;32m");                                               \
        cafe_print("✓ " message "\n");                                         \
        cafe_term("\033[0m");                                                  \
        ++cafe_passing;                                                        \
    }                                                                          \
    for (int cafe_i = cafe_ae_num_hooks[cafe_level] - 1; cafe_i >= 0;          \
         --cafe_i) {                                                           \
        if (setjmp(cafe_return) == 0) {                                        \
            longjmp(cafe_ae_hooks[cafe_i], 1);                                 \
        }                                                                      \
    }

#define Pending(message)                                                       \
    cafe_term("\033[1;36m");                                                   \
    cafe_print("• " message "\n");                                             \
    cafe_term("\033[0m");                                                      \
    ++cafe_pending;

#define BeforeEach(block)                                                      \
    if (setjmp(cafe_be_hooks[cafe_be_num_hooks[cafe_level]++]) != 0) {         \
        do {                                                                   \
            block                                                              \
        } while (0);                                                           \
        longjmp(cafe_return, 1);                                               \
    }

#define AfterEach(block)                                                       \
    if (setjmp(cafe_ae_hooks[cafe_ae_num_hooks[cafe_level]++]) != 0) {         \
        do {                                                                   \
            block                                                              \
        } while (0);                                                           \
        longjmp(cafe_return, 1);                                               \
    }

#define Before(block)                                                          \
    if (setjmp(cafe_b_hooks[cafe_level][cafe_b_num_hooks[cafe_level]++]) !=    \
        0) {                                                                   \
        do {                                                                   \
            block                                                              \
        } while (0);                                                           \
        longjmp(cafe_return, 1);                                               \
    }

#define After(block)                                                           \
    if (setjmp(cafe_a_hooks[cafe_level][cafe_a_num_hooks[cafe_level]++]) !=    \
        0) {                                                                   \
        do {                                                                   \
            block                                                              \
        } while (0);                                                           \
        longjmp(cafe_return, 1);                                               \
    }

#define Cafe(block)                                                            \
    int main(int argc, char **argv) {                                          \
        printf("\n");                                                          \
        double cafe_dtime = cafe_time_ms();                                    \
        do {                                                                   \
            block                                                              \
        } while (0);                                                           \
        printf("\n");                                                          \
        cafe_dtime = cafe_time_ms() - cafe_dtime;                              \
        cafe_print("Results after %d tests (%.0fms)\n",                        \
                   cafe_passing + cafe_pending + cafe_failing, cafe_dtime);    \
        if (cafe_passing) {                                                    \
            cafe_term("\033[1;32m");                                           \
            cafe_print("✓ %d passing\n", cafe_passing);                        \
            cafe_term("\033[0m");                                              \
        }                                                                      \
        if (cafe_pending) {                                                    \
            cafe_term("\033[1;36m");                                           \
            cafe_print("• %d pending\n", cafe_pending);                        \
            cafe_term("\033[0m");                                              \
        }                                                                      \
        if (cafe_failing) {                                                    \
            cafe_term("\033[1;31m");                                           \
            cafe_print("✗ %d failing\n", cafe_failing);                        \
            cafe_term("\033[0m");                                              \
        }                                                                      \
        printf("\n");                                                          \
        return cafe_failing;                                                   \
    }

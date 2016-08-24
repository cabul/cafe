#pragma once

#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#ifndef CAFE_MAX_HOOKS
#define CAFE_MAX_HOOKS 16
#endif

#ifndef CAFE_MAX_LEVELS
#define CAFE_MAX_LEVELS 4
#endif

static int cafe_status = 0;
static int cafe_level = 0;

static int cafe_passing = 0;
static int cafe_failing = 0;
static int cafe_pending = 0;

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

static double cafe_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1e3 + tv.tv_usec * 1e-3;
}

struct cafe_reporter {
    void (*on_start)();
    void (*on_end)(double dt, int passing, int pending, int failing);
    void (*on_section)(char *section);
    void (*on_test)(char *test, char *file, int line, int status);
};

void cafe_on_start();
void cafe_on_end(double dt, int passing, int pending, int failing);
void cafe_on_section(char *section);
void cafe_on_test(char *test, char *file, int line, int status);

static struct cafe_reporter cafe_reporter = {
    .on_start = cafe_on_start,
    .on_end = cafe_on_end,
    .on_section = cafe_on_section,
    .on_test = cafe_on_test, 
};

#define cafe_counter_impl(idx) cafe_counter_##idx
#define cafe_counter(idx) cafe_counter_impl(idx)
#define cafe_loop_impl(idx) cafe_loop_##idx
#define cafe_loop(idx) cafe_loop_impl(idx)

#define cafe_indent(spaces) printf("%*s", spaces, "")

#define Assert(cond)                                                           \
    if (!(cond)) {                                                             \
        cafe_status = -1;                                                      \
        cafe_helper = #cond;                                                   \
        break;                                                                 \
    }

#define Error(message)                                                         \
    cafe_status = -2;                                                          \
    cafe_helper = message;                                                     \
    break;

#define Describe(message)                                                      \
    cafe_r_describe();                                                         \
    if (cafe_reporter.on_section)                                              \
        cafe_reporter.on_section(message);                                     \
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
        if (cafe_status == 0) {                                                \
            ++cafe_passing;                                                    \
        } else {                                                               \
            ++cafe_failing;                                                    \
        }                                                                      \
        if (cafe_reporter.on_test)                                             \
            cafe_reporter.on_test(message, __FILE__, __LINE__, cafe_status);   \
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
    if (cafe_reporter.on_test)                                                 \
        cafe_reporter.on_test(message, __FILE__, __LINE__, 1);                 \
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
        if (cafe_reporter.on_start)                                            \
            cafe_reporter.on_start();                                          \
        double cafe_dtime = cafe_time_ms();                                    \
        cafe_main(argc, argv);                                                 \
        cafe_dtime = cafe_time_ms() - cafe_dtime;                              \
        if (cafe_reporter.on_start)                                            \
            cafe_reporter.on_end();                                            \
        return cafe_failing;                                                   \
    }                                                                          \
                                                                               \
    void cafe_main(int argc, char **argv)

void cafe_on_start() { printf("\n"); }
void cafe_on_end(double dt, int passing, int pending, int failing) {
    printf("\n");
    cafe_indent(2);
    printf("Results after %d tests (%.0fms)\n",
           passing + pending + failing, dtime);
    if (passing) {
        cafe_indent(2);
        printf("✓ %d passing\n", passing);
    }
    if (pending) {
        cafe_indent(2);
        printf("• %d pending\n", pending);
    }
    if (failing) {
        cafe_indent(2);
        printf("✗ %d failing\n", failing);
    }
    printf("\n");
}

void cafe_on_section() {
    cafe_indent(2 + cafe_level * 2);
    printf("%s\n", cafe_info[cafe_level]);
}

void cafe_on_test() {
    cafe_indent(2 + cafe_level * 2);
    switch (cafe_status) {
    case 1:
        printf("• %s\n", cafe_info[cafe_level]);
        break;
    case 0:
        printf("✓ %s\n", cafe_info[cafe_level]);
        break;
    default:
        printf("✗ %s\n", cafe_info[cafe_level]);
        cafe_indent(4 + cafe_level * 2);
        if (cafe_status == -1) {
            printf("» Failed '%s'\n", cafe_helper);
        } else {
            printf("» %s\n", cafe_helper);
        }
        break;
    }
}

void cafe_r_exit_minimal() {
    printf("%d tests (%.0fms)\n", cafe_passing + cafe_pending + cafe_failing,
           cafe_dtime);
    if (cafe_passing) {
        printf("%d passing\n", cafe_passing);
    }
    if (cafe_pending) {
        printf("%d pending\n", cafe_pending);
    }
    if (cafe_failing) {
        printf("%d failing\n", cafe_failing);
    }
}

// if (argc > 1) {                                                        \
        //     if (argv[1][0] == '+') {                                           \
        //         char *reporter = argv[1] + 1;                                  \
        //         if (strcmp(reporter, "minimal") == 0) {                        \
        //             cafe_r_enter = cafe_r_silent;                              \
        //             cafe_r_exit = cafe_r_exit_minimal;                         \
        //             cafe_r_describe = cafe_r_silent;                           \
        //             cafe_r_test = cafe_r_silent;                               \
        //         } else if (strcmp(reporter, "silent") == 0) {                  \
        //             cafe_r_enter = cafe_r_silent;                              \
        //             cafe_r_exit = cafe_r_silent;                               \
        //             cafe_r_describe = cafe_r_silent;                           \
        //             cafe_r_test = cafe_r_silent;                               \
        //         } else {                                                       \
        //             fprintf(stderr, "cafe: Unknown option: '%s'\n",          \
        //                     reporter);                                         \
        //             return -1;                                                 \
        //         }                                                              \
        //     }                                                                  \
        // }                                                                      \

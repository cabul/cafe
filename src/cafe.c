#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

#ifndef CAFE_MAX_LEVELS
#define CAFE_MAX_LEVELS 8
#endif

enum cafe_status {
    CAFE_FAILING = -1,
    CAFE_PASSING = 0,
    CAFE_PENDING = 1,
};

static enum cafe_status status;
static int level = 0;

static int passing = 0;
static int pending = 0;
static int failing = 0;

static int32_t level_set;

static jmp_buf before_hooks[CAFE_MAX_LEVELS];
static int32_t before_set;
static jmp_buf after_hooks[CAFE_MAX_LEVELS];
static int32_t after_set;
static jmp_buf before_each_hooks[CAFE_MAX_LEVELS];
static int32_t before_each_set;
static jmp_buf after_each_hooks[CAFE_MAX_LEVELS];
static int32_t after_each_set;

static jmp_buf return_hook;
static jmp_buf *save_target;

static char *failure_message;

#define CAFE_GOTO(HOOK)                                                        \
    if (setjmp(return_hook) == 0) {                                            \
        longjmp(HOOK, 1);                                                      \
    }

void cafe_save(jmp_buf *target) { save_target = target; }

jmp_buf *cafe_hook_before_each() {
    before_each_set |= 1 << level;
    return before_each_hooks + level;
}
jmp_buf *cafe_hook_after_each() {
    after_each_set |= 1 << level;
    return after_each_hooks + level;
}
jmp_buf *cafe_hook_before() {
    before_set |= 1 << level;
    return before_hooks + level;
}
jmp_buf *cafe_hook_after() {
    after_set |= 1 << level;
    return after_hooks + level;
}

jmp_buf *cafe_return() { return &return_hook; }

void cafe_begin_suite(char *info) {
    ++level;
    printf("%*s%s\n", level * 2, "", info);
    level_set &= ~(1<<level);
}

void cafe_end_suite(char *info) {
    if (level_set & after_set & (1 << level)) {
        CAFE_GOTO(after_hooks[level])
    }
    --level;
}

void cafe_begin_test(char *info) {
    for (int i = 0; i < level; ++i) {
        if ((level_set & (1<<i)) == 0) {
            if (before_set & (1<<i)) {
                CAFE_GOTO(before_hooks[i])
            }
        }
        level_set |= 1 << i;
    }
    for (int i = 0; i < level; ++i) {
        if (before_each_set & (1 << i)) {
            CAFE_GOTO(before_each_hooks[i])
        }
    }
    status = CAFE_PENDING;
}

void cafe_end_test(char *info) {
    char *prefix;
    switch (status) {
    case CAFE_PASSING:
        prefix = "✓";
        ++passing;
        break;
    case CAFE_PENDING:
        prefix = "•";
        ++pending;
        break;
    case CAFE_FAILING:
        prefix = "✗";
        ++failing;
        break;
    }
    printf("%*s  %s %s\n", level * 2, "", prefix, info);
    if (status == CAFE_FAILING) {
        printf("%*s    » %s\n", level * 2, "", failure_message);
    }
    for (int i = level - 1; i >= 0; --i) {
        if (after_each_set & (1 << i)) {
            CAFE_GOTO(after_each_hooks[i])
        }
    }
}

void cafe_check(int err, char *message) {
    if (err) {
        failure_message = message;
        status = CAFE_FAILING;
        CAFE_GOTO(*save_target);
    } else {
        status = CAFE_PASSING;
    }
}

static double get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1e3 + tv.tv_usec * 1e-3;
}

void cafe_main(int argc, char **argv);

int main(int argc, char **argv) {
    printf("\n");
    double dtime = get_time_ms();
    cafe_main(argc, argv);
    dtime = get_time_ms() - dtime;
    printf("\n");
    printf("  Results after %d tests (%.0f ms)\n", passing + pending + failing,
           dtime);
    if (passing) {
        printf("  ✓ %d passing\n", passing);
    }
    if (passing) {
        printf("  • %d pending\n", pending);
    }
    if (passing) {
        printf("  ✗ %d failing\n", failing);
    }
    printf("\n");
    return failing;
}

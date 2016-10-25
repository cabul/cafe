#include "../cafe.h"

#include <stdlib.h>
#include <time.h>

int *vector = NULL;
int size = 10;

int randint(int min, int max) { return min + rand() % (max - min); }

int cmp_asc(const void *a, const void *b) { return *(int *)a - *(int *)b; }
int cmp_desc(const void *a, const void *b) { return *(int *)b - *(int *)a; }

Cafe(main) {
    Describe("qsort()") {
        Before { srand(time(NULL)); } // only executes once

        BeforeEach { // executes before each testbench
            vector = malloc(size * sizeof(int));
            for (int i = 0; i < size; ++i) {
                vector[i] = randint(1, size * 2);
            }
        }

        AfterEach { // executes after each testbench
            free(vector);
            vector = NULL;
        }

        It("should sort in ascending order") {
            qsort(vector, size, sizeof(int), cmp_asc);
            int i;
            for (i = 1; i < size; ++i) {
                if (vector[i - 1] > vector[i])
                    break;
            }
            Assert(i == size);
        }

        It("should sort in descending order") {
            qsort(vector, size, sizeof(int), cmp_desc);
            int i;
            for (i = 1; i < size; ++i) {
                if (vector[i - 1] < vector[i])
                    break;
            }
            Assert(i == size);
        }
    }
}

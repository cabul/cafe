#include "../cafe.h"

int add(int a, int b) { return a + b; }
int sub(int a, int b) { return a - b; }

Cafe(main) {
    Describe("Calculator") {

        Describe("add()") {
            It("should work with positive numbers") {
                Assert(add(2, 2) == 4);
            }
            It("should work with negative numbers") {
                Assert(add(-2, -2) == -4);
            }
            It("should be full of bugs") {
                Fail("This line always fails");
            }
            It("should do magic") {}
        }

        Describe("sub()") {
            It("should work as well..") {
                Assert(add(2,-2) == sub(2,2));
            }
        }

    }
}


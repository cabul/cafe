#include "../cafe.h"

int add(int a, int b) { return a + b; }

Cafe {
    Describe("Calculator") {
        Describe("add()") {
            It("should work with positive numbers") {
                Assert(add(2, 2) == 4);
            }
            It("should work with negative numbers") {
                Assert(add(-2, -2) == -4);
            }
            It("should be full of bugs") {
                Fail("This line always failes");
            }
            It("should do magic") {}
        }
    }
}


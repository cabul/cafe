#include "cafe.h"

#include "../src/hello.h"

Cafe {
    Describe("Hello module") {
        Describe("add_five()") {
            It("should work with positive numbers") {
                Assert(add_five(14) == 19);
            }
            It("should work with negative numbers") {
                Assert(add_five(-3) == 2);
            }
            It("should be full of bugs") { Error("This line always failes") }
            It("should do magic") {}
        }
    }
}

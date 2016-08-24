#include "hello.h"
#include "cafe.h"

Cafe {
    BeforeEach {
        printf("Start Test\n");
    }
    AfterEach {
        printf("End Test\n");
    }
    Describe("Hello module") {
        Describe("add_five()") {
            It("should work with positive numbers") {
                Assert(add_five(4) == 9) }
            It("should work with negative numbers") {
                Assert(add_five(-3) == 2);
            }
            It("should be full of bugs") {
                Error("This line always failes")
            }
            Pending("should do magic") {}
        }
    }
}

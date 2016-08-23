#include "hello.h"
#include "cafe.h"

Cafe {
    Describe("Hello module") {
        Describe("add_five()") {
            It("should work with positive numbers",
                Assert(add_five(4) == 8)
            )
            It("should work with negative numbers",
                Assert(add_five(-3) == 2)
            )
            Pending("should do magic")
        }
    }
}

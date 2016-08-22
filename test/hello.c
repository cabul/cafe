#include "hello.h"
#include "cafe.h"

Cafe(
    Describe("Hello module",
        Describe("add_five()",
            It("should work with positive numbers",
                int x = add_five(4);
                Assert(x == 8)
            )
            It("should work with negative numbers",
                int x = add_five(-3);
                Assert(x == 2)
            )
            Pending("should do magic")
        )
    )
)

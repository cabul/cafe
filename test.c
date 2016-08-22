#include "cafe.h"

Cafe(
    Describe("some simple tests",
        It("should pass",
            Assert(1 != 2)
            Assert(2+2 == 4)
        )
        It("should fail",
            Assert(1 == 2)
        )
    )
)

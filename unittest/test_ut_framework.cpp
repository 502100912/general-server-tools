#include "ut_framework.h"
#include "stdio.h"
G_TEST(test_ut_common) {
    printf("Hello World\n");
}

int main() {
    G_RUNALL();
    return 0;
}

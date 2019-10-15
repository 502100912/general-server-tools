#include "ut_framework.h"

void UnitTestManager::add(TestBase* test) {
    _tests.push_back(test);
}

void UnitTestManager::run_all() {
    for (auto test : _tests) {
        test->test_body();
    }
}

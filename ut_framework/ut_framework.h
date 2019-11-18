#ifndef UNITTEST_FRAMEWORK_H
#define UNITTEST_FRAMEWORK_H
#include <vector>

//Parent class of a test case
class TestBase {
public:
    virtual void test_body() = 0;
    virtual const char* name() {return "undefiend";}
};

class UnitTestManager {
public:
    static UnitTestManager* instance() {
        static UnitTestManager singleton;
        return &singleton;
    }

    void add(TestBase* test);

    void run_all(); 
private:
    std::vector<TestBase*> _tests;
};


//##:token concatenation. concatenate two tokens
//#:stringification, x=123,#x->"123"
#define G_TEST(test_name)                           \ 
class test_name : public TestBase{                  \
public:                                             \
    test_name() {                                   \
        UnitTestManager::instance()->add(this);     \
    }                                               \
    void test_body();                               \
    const char* name() {return #test_name;}         \
};                                                  \
static test_name test_name;                                \ 
void test_name::test_body()


        
#define G_RUNALL()                                  \
UnitTestManager::instance()->run_all()              \


#endif

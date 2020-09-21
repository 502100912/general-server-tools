#pragma once
#include <sstream>
#include <cassert>
#include <iostream>

#define EXPECT_EQ(v1, v2)\
ExpectEQImpl(v1 ,v2, __FILE__, __LINE__) 

#define EXPECT_NE(v1, v2)\
ExpectNEImpl(v1 ,v2, __FILE__, __LINE__) 

template<typename T1, typename T2>
int ExpectEQImpl(const T1& t1, const T2& t2, const char* file, int line) {
    if (t1 == t2) return 0;
    std::stringstream expect_fail_info; 
    std::cerr << "EXPECT_EQ False, [" << file << "]" << "[" << line << "] \n" <<
        "v1:" << std::to_string(t1) << "\n" <<
        "v2:" << std::to_string(t2) << "\n";
    return -1;
}

template<typename T1, typename T2>
int ExpectNEImpl(const T1& t1, const T2& t2, const char* file, int line) {
    if (t1 != t2) return 0;
    std::stringstream expect_fail_info; 
    std::cerr << "EXPECT_NE False, where " << file << line << "\n" << 
        "v1:" << std::to_string(t1) << "\n" <<
        "v2:" << std::to_string(t2) << "\n";
    return -1;
}

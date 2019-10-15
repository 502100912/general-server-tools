all : output_dir \
    test_ut_framework \
    clean
.PHONY : all

#for unittest
output_dir: 
	rm -rf ./output
	mkdir -p ./output/test

test_ut_framework :  test_ut_framework.o ut_framework.o output_dir
	g++ ut_framework.o test_ut_framework.o -o ./output/test/test_ut_framework -std=c++11

test_ut_framework.o : unittest/test_ut_framework.cpp ut_framework/ut_framework.h ut_framework/ut_framework.cpp 
	g++ -c unittest/test_ut_framework.cpp -I. -I./ut_framework  -o test_ut_framework.o -std=c++11

ut_framework.o : ut_framework/ut_framework.h ut_framework/ut_framework.cpp
	g++ -c ut_framework/ut_framework.cpp -o ut_framework.o -std=c++11

clean : 
	rm -rf *o

#资料
#https://cloud.tencent.com/developer/article/1065286

CXX = g++
CXXFLAGS = -g -O2 -std=c++0x -Wno-deprecated
##CXXFLAGS = -g -O2 -Wc++17-extensions -Wno-deprecated

#global include path
INCLUDE = -I./src -I./ut_framework

#for gserver source code
DIRS := ./src ./ut_framework
SOURCES := $(foreach d,$(DIRS),$(wildcard $(addprefix $(d)/*,cpp)))
OBJS := $(addsuffix .o, $(basename $(SOURCES))) 
LIB :=gserverlib
LD := -L. -l$(LIB)

#for uinttest
TEST_DIRS := ./unittest
TEST_SOURCES := $(foreach d,$(TEST_DIRS),$(wildcard $(addprefix $(d)/*,cpp)))
TEST_OBJS := $(addsuffix .o, $(basename $(TEST_SOURCES))) 
TESTS_EXE := $(basename $(TEST_SOURCES))

.PHONY:all clean

all:$(LIB) $(TESTS_EXE) 

#compile gserver source code to static lib
$(LIB):$(OBJS)
	ar crs lib$@.a $(OBJS)

#compile all cpp to .o
%.o:%.cpp
	$(CXX) -c $(INCLUDE) $(CXXFLAGS) $< -o $@

#comile all unittest and link lib,output executables
$(TESTS_EXE):$(TEST_OBJS)
	$(CXX) $(CXXFLAG) $@.o -o ./output/test/$(notdir $@) $(LD)


#clean middle files
clean: 
	rm -rf ./output
	rm -rf ./src/*.o ./unittest/*.o ./ut_framework/*.o
	rm -rf ./lib*a
	mkdir -p ./output/test 

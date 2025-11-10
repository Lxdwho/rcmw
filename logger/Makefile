src = $(wildcard *.cpp)
obj = $(patsubst %.cpp, %.out, $(src))
ocj = $(patsubst %.cpp, %, $(src))


ALL:test_logger.out

test_logger.out:test_logger.cpp
	g++ test_logger.cpp logger.cpp -o test_logger.out

clear:
	-rm -rf *.out $(ocj)

.PHONY:clear
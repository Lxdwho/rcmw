CC			:= 	g++
CFLAGS		:= 	-std=c++14
DEBUG		:= 	-g
INCLUDE		:= 	-I/home/me/cmw_ws/rcmw/			\
            	-I/home/me/cmw_ws/rcmw/m_test/	\
                -I/home/me/cmw_ws/rcmw/base/    \
                -I/home/me/googletest/install/include/

ThirdPartyLibPath :=	-L/home/me/Fast-DDS/install/lib \
						-L/home/me/googletest/install/lib

ThirdPartyLib	:= 	-lgtest -lgtest_main -lpthread -latomic \
					-lfastrtps -lfastcdr -lfoonathan_memory-0.7.3

SRCS := $(wildcard *.cpp)
OUTS := $(pastsubst %.cpp, %.out, $(SRCS))
TARS := $(pastsubst %.cpp, %, $(SRCS))

ALL:$(TARS)
%.out : %.cpp
	$(CC) $(INCLUDE) $(DEBUG) -o $@ $^ \
	$(ThirdPartyLibPath) \
	$(ThirdPartyLib)

.PHONY: clean
clean:
	rm -rf *.out *.o $(TARS)
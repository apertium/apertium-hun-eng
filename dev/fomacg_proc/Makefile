INCFLAGS = -I/usr/local/include/ -I../../src/ -I.
CPPFLAGS = -g -ggdb $(INCFLAGS) -Wall -O3
CPP = g++
CXX = g++
LINKFLAGS = -lfoma
all: fomacg_proc

fomacg_proc: *.cpp
	     $(CPP) $(CPPFLAGS) *.cpp -o $@ $(LINKFLAGS)

clean:
	rm -f $(patsubst %.cpp, %, $(wildcard *.cpp))

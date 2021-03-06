LINK_AND_COMPILER_FLAGS=-g 
CXXFLAGS=$(LINK_AND_COMPILER_FLAGS) -Wall -O3
LDFLAGS=$(LINK_AND_COMPILER_FLAGS)
CC=g++

calc:	calc.o xbmath.o
	$(CXX) -o $@ $^ $(LDFLAGS) $(LDLIBS)    

clean:
	rm -rf *.o calc

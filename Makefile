PROG=sudoku
CXXFLAGS=-std=c++14 -Wall -Wextra -pedantic -MMD
CXX=g++

.PHONY: clean

$(PROG): sudoku.o
	$(CXX) $(CXXFLAGS) -o $@ $^

-include *.d

clean:
	rm *.o $(PROG)

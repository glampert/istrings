
BIN_TARGET = istrings
SRC_FILE   = improved_strings.cpp
CXXFLAGS   = -std=c++11 -O2 -Wall -Wextra -pedantic

all:
	$(CXX) $(CXXFLAGS) $(SRC_FILE) -o $(BIN_TARGET)

clean:
	rm -f *.o
	rm -f $(BIN_TARGET)


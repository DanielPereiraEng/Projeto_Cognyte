CXX = g++
CXXFLAGS = -std=c++11 -Wall
SRC = your_source_files.cpp
OBJ = $(SRC:.cpp=.o)
EXECUTABLE = your_executable_name

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ)
    $(CXX) $(CXXFLAGS) -o $@ $^

clean:
    rm -f $(EXECUTABLE) $(OBJ)

.cpp.o:
    $(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all clean

CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic -std=c++11 -g -fsanitize=address -Iinclude -c
LDFLAGS =  -fsanitize=address
LBLIBS = -lconfig++ -pthread

SRC = $(wildcard ./src/*.cpp) 
SRC_HTTP = $(wildcard ./src/http/*.cpp)
DIR = $(notdir $(SRC))
DIR_HTTP = $(notdir $(SRC_HTTP))
OBJ = $(addprefix bin/,$(DIR:.cpp=.o)) $(addprefix bin/http/,$(DIR_HTTP:.cpp=.o))
EXEC = httpserver

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CXX) -o $@ $(OBJ) $(LBLIBS) $(LDFLAGS)

bin/http/%.o : src/http/%.cpp
	$(CXX) -o $@ $^ $(CXXFLAGS)

bin/%.o : src/%.cpp
	$(CXX) -o $@ $^ $(CXXFLAGS)


clean:
	rm -rf $(OBJ) $(EXEC)
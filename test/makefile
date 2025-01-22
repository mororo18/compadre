CXXFLAGS = -O3 -Wall -Wextra -pedantic -std=c++2b -I ../external/ -I ../src -g
HXX = ../src/compadre.hpp
OBJ = ../compadre.o

TARGET = run.out

test: $(TARGET) 
	./$(TARGET)

$(TARGET): main.cpp $(HXX) $(OBJ)
	$(CXX) $< -o $@ $(OBJ) $(CXXFLAGS)

$(OBJ):
	make -C ../
	$(CP) $(OBJ) .

clean:
	$(RM) $(TARGET)

.PHONY: clean test

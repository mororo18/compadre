CXXFLAGS = -O3 -Wall -Wextra -pedantic -std=c++2b -I ../external/ -I ../src -g
HXX = ../src/compadre.hpp
OBJ = ../compadre.o
OUTBIT_OBJ = BitBuffer.o

TARGET = run.out

test: $(TARGET) 
	./$(TARGET)

$(TARGET): main.cpp $(HXX) $(OBJ) $(OUTBIT_OBJ)
	$(CXX) $< -o $@ $(OBJ) $(OUTBIT_OBJ) $(CXXFLAGS)

$(OBJ):
	make -C ../

$(OUTBIT_OBJ):
	make -C ../external/outbit M=$(shell pwd) lib

clean:
	$(RM) $(TARGET)
	make -C ../external/outbit M=$(shell pwd) clean-lib

.PHONY: clean test

CXXFLAGS = -O3 -Wall -Wextra -pedantic -std=c++2b -g
OUTBIT_OBJ = BitBuffer.o
COMP_OBJ = compadre.o
EXTERNAL_DIR = external/
TARGET = compadre
TEST_DIR = test/

$(TARGET): src/main.cpp $(OUTBIT_OBJ) $(COMP_OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $< $(OUTBIT_OBJ) $(COMP_OBJ) -I$(EXTERNAL_DIR)

$(COMP_OBJ): src/compadre.cpp src/compadre.hpp
	$(CXX) $(CXXFLAGS) -I$(EXTERNAL_DIR) -c $<

$(OUTBIT_OBJ):
	+make -C $(EXTERNAL_DIR)/outbit M=$(shell pwd) lib

all: $(TARGET) test

test: $(COMP_OBJ)
	+make -C $(TEST_DIR)

clean:
	$(RM) $(TARGET)
	$(RM) $(COMP_OBJ)
	+make -C $(TEST_DIR) clean
	+make -C $(EXTERNAL_DIR)/outbit M=$(shell pwd) clean-lib

.PHONY: clean test

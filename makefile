CXXFLAGS = -Wall -Wextra -pedantic -std=c++2b -g
OUTBIT_OBJ = BitBuffer.o
EXTERNAL_DIR = external/
TARGET = compadre

$(TARGET): src/main.cpp $(OUTBIT_OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $< $(OUTBIT_OBJ) -I$(EXTERNAL_DIR)

$(OUTBIT_OBJ):
	make -C $(EXTERNAL_DIR)/outbit M=$(shell pwd) lib

clean:
	rm $(TARGET)
	make -C $(EXTERNAL_DIR)/outbit M=$(shell pwd) clean-lib

.PHONY: clean


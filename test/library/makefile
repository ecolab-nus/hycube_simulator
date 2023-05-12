# Makefile

# Paths
INCLUDE_PATH = /home/dhananjaya/workspace/Morpher/hycube_simulator/src
LIBRARY_PATH = /home/dhananjaya/workspace/Morpher/hycube_simulator/src/build
LIBRARY_NAME = hycube_simulator

# Compiler
CXX = g++

# Source file
SOURCE = array_add.cpp

# Output binary
BINARY = main

# Flags
CXXFLAGS = -I$(INCLUDE_PATH)
LDFLAGS = -L$(LIBRARY_PATH) -l$(LIBRARY_NAME)

$(BINARY): $(SOURCE)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(BINARY)


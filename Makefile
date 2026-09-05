CXX = g++
CXXFLAGS = -std=c++20 -O2 -g -MMD -MP -I src
CXXFLAGS += -Wall -Werror -Wextra \
            -Wformat-security \
            -Wpointer-arith \
            -Woverflow \
            -Wvla \
            -Wunused \
            -Wnon-virtual-dtor \
            -Woverloaded-virtual \
            -Wno-unused-parameter \
            -Wno-missing-field-initializers

TARGET = build/adamastor

SRC  = src/main.cc

OBJ = $(SRC:src/%.cc=build/%.o)
DEP = $(OBJ:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

build/%.o: src/%.cc | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

build:
	mkdir -p build

clean:
	rm -rf build/

-include $(DEP)

.PHONY: all clean

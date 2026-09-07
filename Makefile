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

CLANG_FORMAT ?= clang-format
CLANG_TIDY   ?= clang-tidy

SRC  = src/main.cc
SRC  += src/parse-options.cc
SRC  += src/replay-util.cc

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

fmt:
	$(CLANG_FORMAT) -i src/*.cc src/*.h

fmt-check:
	$(CLANG_FORMAT) --dry-run -Werror src/*.cc src/*.h

tidy:
	$(CLANG_TIDY) $(SRC) -- -std=c++20 -I src

-include $(DEP)

.PHONY: all clean fmt fmt-check tidy

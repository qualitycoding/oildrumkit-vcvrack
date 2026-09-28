# FROZEN — DO NOT MODIFY (tests/FROZEN_MANIFEST.sha256)
# Builds and runs the C++ test binaries with Rack's own compiler flags (C-008: Rack compile.mk
# -O3 -funsafe-math-optimizations -fno-omit-frame-pointer -march=nehalem) plus -std=c++17 (D-008).
CXX      ?= g++
ENGINE   ?= ../engine/Source
CXXFLAGS := -std=c++17 -O3 -funsafe-math-optimizations -fno-omit-frame-pointer -march=nehalem \
            -Wall -Wextra -Wno-unused-parameter -I../src/core -I$(ENGINE)
BIN      := build
TESTS    := test_mapping test_trigger test_reference test_fidelity test_operational test_robustness test_perf
CORE_OBJ := $(BIN)/KitCore.o

all: $(addprefix $(BIN)/,$(TESTS))

$(BIN)/KitCore.o: ../src/core/KitCore.cpp ../src/core/KitCore.hpp
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN)/%: %.cpp harness.hpp reference.hpp gen.hpp $(CORE_OBJ)
	@mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) $< $(CORE_OBJ) -o $@

run: all
	@rc=0; for t in $(TESTS); do echo "== $$t"; ./$(BIN)/$$t || rc=1; done; exit $$rc

clean:
	rm -rf $(BIN)

.PHONY: all run clean

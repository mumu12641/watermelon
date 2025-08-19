# opt -S -load-pass-plugin=lib/libconstant_prop_pass.so -passes="constant-prop-pass" tests/test.ll -o tests/test_pass_1.ll
# opt -S -load-pass-plugin=lib/libdce_pass.so -passes="dce-pass" tests/test_pass_1.ll -o tests/test_pass_2.ll

CXX = clang++
CXXFLAGS = -O3 -fPIC -fno-rtti $(shell llvm-config --cxxflags)
LDFLAGS = -shared $(shell llvm-config --ldflags)

SOURCES = $(wildcard src/*.cpp)
TARGETS = $(patsubst src/%.cpp,lib/lib_%.so,$(SOURCES))
OBJECTS = $(patsubst src/%.cpp,build/%.o,$(SOURCES))

all: $(TARGETS)

build_dir:
	@mkdir -p build lib

lib/lib%.so: build/%.o | build_dir
	$(CXX) $< $(LDFLAGS) -o $@

build/%.o: src/%.cpp | build_dir
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGETS) $(OBJECTS)
	rm -rf build lib

.PHONY: all clean build_dir show-targets
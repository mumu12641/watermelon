
CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -O2 -g
INCLUDES = -Iinclude
LLVMIRFLAGS = -S -emit-llvm -O0 -std=c++17 -Xclang -disable-O0-optnone
CHECKER = valgrind --undef-value-errors=no  --leak-check=full 

SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
LIB_DIR = lib
TARGET = ./build/output

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
LLVMIRS = $(SOURCES:$(SRC_DIR)/%.cpp=$(LIB_DIR)/%.ll)
HEADERS = $(wildcard $(INCLUDE_DIR)/*.hpp)

all: $(BUILD_DIR) $(LIB_DIR) $(TARGET) llvm-ir

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(LIB_DIR)/%.ll: $(SRC_DIR)/%.cpp $(HEADERS)
	clang++ $(LLVMIRFLAGS)  $< -o $@

llvm-ir: $(LIB_DIR) $(LLVMIRS)

clean:
	rm -rf $(BUILD_DIR) $(LIB_DIR) $(TARGET)
check:
	$(CHECKER) ./build/output

.PHONY: all compile llvm-ir clean 

# 一行命令清理
# clang++ -S -emit-llvm -O0 -fno-rtti -fno-exceptions gc_wrapper.cpp -o temp.ll && \
# sed -E 's/dso_local //g; s/noundef //g; s/ #[0-9]+//g; s/nonnull //g; s/align [0-9]+ //g; s/dereferenceable\([0-9]+\) //g; /^attributes #/d; /^!/d; /^source_filename/d; /^target/d; /^$/d' temp.ll > gc_clean.ll && \
# rm temp.ll

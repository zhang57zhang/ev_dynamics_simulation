# Make文件模板（可选，用于简化命令）

.PHONY: all build clean test install docs help

# 默认目标
all: build

# 构建项目
build:
	@echo "Building project..."
	@if [ -f build.sh ]; then \
		./build.sh; \
	else \
		mkdir -p build && cd build && cmake .. && make -j$$(nproc); \
	fi

# 清理构建
clean:
	@echo "Cleaning build..."
	@rm -rf build

# 运行测试
test: build
	@echo "Running tests..."
	@cd build && ctest --output-on-failure

# 安装
install: build
	@echo "Installing..."
	@cd build && cmake --install .

# 生成文档
docs:
	@echo "Generating documentation..."
	@doxygen Doxyfile

# 格式化代码
format:
	@echo "Formatting code..."
	@find src -name '*.cpp' -o -name '*.h' | xargs clang-format -i

# 静态分析
analyze:
	@echo "Running static analysis..."
	@cppcheck --enable=all src/

# 帮助
help:
	@echo "Available targets:"
	@echo "  all      - Build project (default)"
	@echo "  build    - Build project"
	@echo "  clean    - Clean build directory"
	@echo "  test     - Run tests"
	@echo "  install  - Install project"
	@echo "  docs     - Generate documentation"
	@echo "  format   - Format source code"
	@echo "  analyze  - Run static analysis"
	@echo "  help     - Show this help message"

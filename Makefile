# ══════════════════════════════════════════════════════════════════
#  Nodevia (NDV) — Makefile
#  Build: make           →  compile release binary
#         make debug     →  compile with debug symbols + sanitizers
#         make run       →  build & run
#         make clean     →  remove build artifacts
#         make rebuild   →  clean + build
#         make check     →  verify all source files exist
# ══════════════════════════════════════════════════════════════════

# ── Toolchain ────────────────────────────────────────────────────
CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -Iinclude

# Release flags (optimised, strip debug info)
RELEASE_FLAGS := -O2 -DNDEBUG

# Debug flags (sanitizers, no optimisation, full symbols)
DEBUG_FLAGS   := -O0 -g3 -fsanitize=address,undefined -fno-omit-frame-pointer

# ── Paths ────────────────────────────────────────────────────────
SRC_DIR  := src
OBJ_DIR  := build/obj
BIN_DIR  := build

TARGET       := $(BIN_DIR)/nodevia
TARGET_DEBUG := $(BIN_DIR)/nodevia_debug

# ── Sources & Objects ────────────────────────────────────────────
SRCS := $(SRC_DIR)/main.cpp        \
        $(SRC_DIR)/block.cpp       \
        $(SRC_DIR)/blockchain.cpp  \
        $(SRC_DIR)/pow.cpp         \
        $(SRC_DIR)/utils.cpp       \
        $(SRC_DIR)/transaction.cpp \
        $(SRC_DIR)/wallet.cpp      \
        $(SRC_DIR)/seed.cpp

OBJS       := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o,       $(SRCS))
OBJS_DEBUG := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%_debug.o, $(SRCS))

# ── Default target ───────────────────────────────────────────────
.PHONY: all
all: $(TARGET)
	@echo ""
	@echo "  ✔  Build complete → $(TARGET)"
	@echo ""

# ── Link release binary ──────────────────────────────────────────
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(RELEASE_FLAGS) $^ -o $@

# ── Link debug binary ────────────────────────────────────────────
$(TARGET_DEBUG): $(OBJS_DEBUG) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) $^ -o $@

# ── Compile release objects ──────────────────────────────────────
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@echo "  CC  $<"
	$(CXX) $(CXXFLAGS) $(RELEASE_FLAGS) -MMD -MP -c $< -o $@

# ── Compile debug objects ────────────────────────────────────────
$(OBJ_DIR)/%_debug.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@echo "  CC  $< [debug]"
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -MMD -MP -c $< -o $@

# ── Auto dependency tracking (.d files) ─────────────────────────
-include $(OBJS:.o=.d)
-include $(OBJS_DEBUG:.o=.d)

# ── Create output directories ────────────────────────────────────
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# ── Convenience targets ──────────────────────────────────────────
.PHONY: debug
debug: $(TARGET_DEBUG)
	@echo ""
	@echo "  ✔  Debug build complete → $(TARGET_DEBUG)"
	@echo ""

.PHONY: run
run: all
	@echo "  ▶  Starting Nodevia...\n"
	@$(TARGET)

.PHONY: run-debug
run-debug: debug
	@echo "  ▶  Starting Nodevia [debug]...\n"
	@$(TARGET_DEBUG)

.PHONY: rebuild
rebuild: clean all

.PHONY: check
check:
	@echo "Checking source files..."
	@for f in $(SRCS); do \
		if [ -f $$f ]; then echo "  ✔  $$f"; \
		else echo "  ✘  MISSING: $$f"; fi; \
	done
	@echo "Done."

.PHONY: clean
clean:
	@echo "  🗑  Cleaning build artifacts..."
	rm -rf $(OBJ_DIR) $(BIN_DIR) *.dat
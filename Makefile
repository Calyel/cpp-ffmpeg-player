# Autor: Calyel Junio de Andrade dos Santos
# Makefile para compilação, execução e testes do cpp-ffmpeg-player

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2
INCLUDES := -Iinclude

BUILD_DIR := build
BIN_DIR   := bin

TARGET      := $(BIN_DIR)/cpp-ffmpeg-player
TEST_TARGET := $(BIN_DIR)/test_runner

LIB_SRCS := src/ProcessRunner.cpp src/MediaInfo.cpp src/VideoPlayer.cpp
LIB_OBJS := $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(LIB_SRCS))

MAIN_OBJ := $(BUILD_DIR)/main.o
TEST_OBJ := $(BUILD_DIR)/test_main.o

SAMPLE_VIDEO ?= assets/sample.mp4

.PHONY: all test info json codec play run clean help

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(TEST_OBJ): tests/test_main.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(TARGET): $(LIB_OBJS) $(MAIN_OBJ) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(TEST_TARGET): $(LIB_OBJS) $(TEST_OBJ) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET)

info: $(TARGET)
	./$(TARGET) --info $(SAMPLE_VIDEO)

json: $(TARGET)
	./$(TARGET) --json $(SAMPLE_VIDEO)

codec: $(TARGET)
	./$(TARGET) --codec $(SAMPLE_VIDEO)

play: $(TARGET)
	./$(TARGET) --play $(SAMPLE_VIDEO)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

help:
	@echo "Alvos disponiveis no Makefile:"
	@echo "  make          - Compila o executavel principal em $(TARGET)"
	@echo "  make test     - Compila e executa a suite de testes automatizados"
	@echo "  make info     - Exibe o relatorio completo de metadados de $(SAMPLE_VIDEO)"
	@echo "  make json     - Exporta os metadados de $(SAMPLE_VIDEO) em JSON"
	@echo "  make codec    - Exibe apenas os codecs de $(SAMPLE_VIDEO)"
	@echo "  make play     - Reproduz $(SAMPLE_VIDEO) com ffplay"
	@echo "  make run      - Inicia o programa em modo interativo"
	@echo "  make clean    - Remove os diretorios build/ e bin/"

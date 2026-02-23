CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LDFLAGS  = -lpthread

TARGET = apple-juice
TESTES = testes

TESTES_SRC = testes/testes.cpp
TESTES_BIN = testes/testes

# Detecta o sistema operacional
UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
    # Tenta usar pkg-config; se não encontrar raylib, usa fallback manual
    RAYFLAGS := $(shell pkg-config --cflags --libs raylib 2>/dev/null || echo "-lraylib -lGL -lm -ldl -lrt -lX11")
endif

ifeq ($(UNAME), Darwin)
    # macOS: raylib geralmente instalado via Homebrew
    RAYFLAGS := $(shell pkg-config --cflags --libs raylib 2>/dev/null || echo "-lraylib -framework OpenGL -framework Cocoa -framework IOKit")
endif

# Windows (MinGW)
ifeq ($(OS), Windows_NT)
    RAYFLAGS  = -lraylib -lopengl32 -lgdi32 -lwinmm
    TARGET   := $(TARGET).exe
    TESTES   := $(TESTES).exe
endif

all: $(TARGET)

$(TARGET): apple-juice.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(RAYFLAGS) $(LDFLAGS)

# roda o simulador
run: $(TARGET)
	./$(TARGET)


# Compila testes
$(TESTES_BIN): $(TESTES_SRC)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)


# Executa testes
test: $(TESTES_BIN)
	./$(TESTES_BIN)


# apagando binários
clean:
	rm -f $(TARGET) $(TESTES)


.PHONY: all run testes test clean
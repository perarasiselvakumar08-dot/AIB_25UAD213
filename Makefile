# Makefile for Transaction Processing System
# Compile with: make
# Clean with: make clean
# Help with: make help

CC = gcc
CFLAGS = -std=c99 -Wall -pedantic
TARGET = transaction_system
SOURCE = transaction_system.c
OBJECT = transaction_system.o

.PHONY: all clean help

all: $(TARGET)

$(TARGET): $(OBJECT)
	@echo "[LINK] Building executable: $(TARGET)"
	$(CC) $(OBJECT) -o $(TARGET)
	@echo "[SUCCESS] Executable created: ./$(TARGET)"

$(OBJECT): $(SOURCE)
	@echo "[COMPILE] Compiling: $(SOURCE)"
	$(CC) $(CFLAGS) -c $(SOURCE) -o $(OBJECT)
	@echo "[SUCCESS] Object file created: $(OBJECT)"

clean:
	@echo "[CLEAN] Removing build artifacts..."
	rm -f $(OBJECT) $(TARGET)
	@echo "[SUCCESS] Clean complete."

help:
	@echo "Transaction Processing System - Build Commands"
	@echo ""
	@echo "  make        - Compile the program"
	@echo "  make clean  - Remove compiled files"
	@echo "  make help   - Display this help message"
	@echo ""
	@echo "After compilation, run with: ./$(TARGET)"

.PHONY: run
run: $(TARGET)
	@echo "[RUN] Starting program..."
	./$(TARGET)

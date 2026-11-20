CXX = g++
CXXFLAGS = -std=c++14 -Wall -Wextra -Iinclude -O2

SRCDIR = src
INCDIR = include
BINDIR = bin

SRCS = $(SRCDIR)/Item.cpp \
       $(SRCDIR)/PerishableItem.cpp \
       $(SRCDIR)/ElectronicsItem.cpp \
       $(SRCDIR)/Transaction.cpp \
       $(SRCDIR)/FileStorage.cpp \
       $(SRCDIR)/InventoryManager.cpp \
       $(SRCDIR)/ConsoleUI.cpp

MAIN = $(SRCDIR)/main.cpp
TEST_SRC = tests/demo_runner.cpp

TARGET = $(BINDIR)/inventory_app.exe
TEST_TARGET = $(BINDIR)/demo_runner.exe

all: $(TARGET)

$(TARGET): $(SRCS) $(MAIN)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

test: $(TEST_TARGET)
	@./$(TEST_TARGET)

$(TEST_TARGET): $(SRCS) $(TEST_SRC)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(filter-out $(SRCDIR)/ConsoleUI.cpp, $(SRCS)) $(TEST_SRC) -o $@

clean:
	@rm -rf $(BINDIR) *.o *.exe

.PHONY: all test clean

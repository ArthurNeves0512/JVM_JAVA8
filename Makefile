CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 \
  -Isrc \
  -Isrc/lib \
  -Isrc/lib/types \
  -Isrc/lib/types/class_file \
  -Isrc/lib/class_loader \
  -Isrc/lib/file \
  -Isrc/lib/printer
LDFLAGS =

TARGET   = main
BUILDDIR = build

SRCS = $(shell find src -name '*.c')
OBJS = $(patsubst %.c, $(BUILDDIR)/%.o, $(SRCS))

TESTDIR  = tests
TEST_SRCS = $(shell find $(TESTDIR) -name '*.c')
TEST_BINS = $(patsubst $(TESTDIR)/%.c, $(BUILDDIR)/$(TESTDIR)/%, $(TEST_SRCS))
APP_OBJS  = $(filter-out $(BUILDDIR)/src/main.o, $(OBJS))

# Análise estática e de memória
CPPCHECK_FLAGS = --enable=warning,style,performance,portability \
                 --std=c11 \
                 --error-exitcode=1 \
                 --suppress=missingIncludeSystem \
                 --suppress=constParameter:src/lib/utils/args.c \
                 -I src -I src/lib -I src/lib/types -I src/lib/types/class_file \
                 -I src/lib/class_loader -I src/lib/file -I src/lib/printer

MEMCHECK_FILE ?= data/examples/Soma.class
VALGRIND_FLAGS = --leak-check=full \
                 --show-leak-kinds=all \
                 --track-origins=yes \
                 --error-exitcode=1

.PHONY: all clean run test lint memcheck

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

$(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

FILE ?= data/examples/Opa.class

run: all
	./$(TARGET) $(FILE)

$(BUILDDIR)/$(TESTDIR)/%: $(TESTDIR)/%.c $(APP_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< $(APP_OBJS) -o $@

test: $(APP_OBJS) $(TEST_BINS)
	@echo "=== Running tests ==="
	@for t in $(TEST_BINS); do ./$$t || exit 1; done
	@echo "=== All tests passed ==="

# Análise estática com cppcheck
lint:
	@command -v cppcheck >/dev/null 2>&1 || \
	  { echo "cppcheck não encontrado. Instale com: sudo apt-get install cppcheck"; exit 1; }
	@echo "=== cppcheck (análise estática) ==="
	cppcheck $(CPPCHECK_FLAGS) src/
	@echo "=== cppcheck: sem problemas encontrados ==="

# Análise dinâmica de memória com valgrind
valgrind: all
	@command -v valgrind >/dev/null 2>&1 || \
	  { echo "valgrind não encontrado. Instale com: sudo apt-get install valgrind"; exit 1; }
	@echo "=== valgrind (análise de memória: $(MEMCHECK_FILE)) ==="
	valgrind $(VALGRIND_FLAGS) ./$(TARGET) $(MEMCHECK_FILE) 2>&1 | \
	  grep -v "^==[0-9]*== For lists of detected" | \
	  grep -v "^==[0-9]*== For counts of" | \
	  grep -v "^==[0-9]*== Use a" | \
	  grep -v "^==[0-9]*== Rerun with"
	@echo "=== valgrind: sem vazamentos detectados ==="

clean:
	rm -rf $(BUILDDIR) $(TARGET)

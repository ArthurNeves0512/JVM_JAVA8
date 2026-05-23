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

.PHONY: all clean run test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

$(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

FILE ?= data/examples/A.class

run: all
	./$(TARGET) $(FILE)

$(BUILDDIR)/$(TESTDIR)/%: $(TESTDIR)/%.c $(APP_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< $(APP_OBJS) -o $@

test: $(APP_OBJS) $(TEST_BINS)
	@echo "=== Running tests ==="
	@for t in $(TEST_BINS); do ./$$t || exit 1; done
	@echo "=== All tests passed ==="

clean:
	rm -rf $(BUILDDIR) $(TARGET)

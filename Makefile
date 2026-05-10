CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 \
  -Isrc \
  -Isrc/lib \
  -Isrc/lib/types \
  -Isrc/lib/types/class_file \
  -Isrc/lib/class_loader \
  -Isrc/lib/file
LDFLAGS =

TARGET   = main
BUILDDIR = build

SRCS = $(shell find src -name '*.c')
OBJS = $(patsubst %.c, $(BUILDDIR)/%.o, $(SRCS))

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

$(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

FILE ?= data/examples/A.class

run: all
	./$(TARGET) $(FILE)

clean:
	rm -rf $(BUILDDIR) $(TARGET)

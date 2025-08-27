CC        = gcc
CFLAGS    = -std=c90 -Wall -Wextra -g -Iinclude
LDFLAGS   =
SRCDIR    = src
BUILDDIR  = build
TARGET    = compiler

SOURCES   = $(wildcard $(SRCDIR)/*.c)
OBJECTS   = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SOURCES))

.PHONY: all clean

all: $(BUILDDIR) $(TARGET)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Compile each .c into build/*.o
$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link objects into the executable
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf $(BUILDDIR) $(TARGET)


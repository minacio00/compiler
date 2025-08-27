CC        = gcc
CFLAGS    = -std=c90 -Wall -Wextra -g -Iinclude
LDFLAGS   =
SRCDIR    = src
BUILDDIR  = build
TARGET    = compiler

SOURCES   = $(wildcard $(SRCDIR)/*.c)
OBJECTS   = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SOURCES))

.PHONY: all clean test

all: $(BUILDDIR) $(TARGET)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Compile each .c into build/*.o
$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Link objects into the executable
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

TEST_SOURCES := $(wildcard tests/*.src)
MEM_ALERT := tests/memory_usage_alert.src
MEM_ERROR := tests/memory_usage_error.src
TESTS := $(filter-out $(MEM_ALERT) $(MEM_ERROR),$(TEST_SOURCES))

test: $(TARGET)
	@for t in $(TESTS); do \
		echo "==> $$t"; \
		./$(TARGET) $$t > $$t.log 2>&1 || true; \
	done
	@for t in $(MEM_ALERT); do \
		echo "==> $$t (MM_LIMIT=3000)"; \
		MM_LIMIT=3000 ./$(TARGET) $$t > $$t.log 2>&1 || true; \
	done
	@for t in $(MEM_ERROR); do \
		echo "==> $$t (MM_LIMIT=2000)"; \
		MM_LIMIT=2000 ./$(TARGET) $$t > $$t.log 2>&1 || true; \
	done

clean:
	rm -rf $(BUILDDIR) $(TARGET)

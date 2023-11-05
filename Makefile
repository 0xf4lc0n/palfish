CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Werror -O2 -fstack-protector-all -fPIE -fPIC -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Wformat-nonliteral -fno-strict-aliasing
TARGET = target/blowfish

SRCDIR = src
OBJDIR = obj

# List of source files
SRCS = $(wildcard $(SRCDIR)/*.c)

# List of tests source files

# Object files
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

# Dependencies
DEPS = $(wildcard $(SRCDIR)/*.h)

# Tests
TESTDIR = tests
TEST_SRCS = $(wildcard $(TESTDIR)/*.c)
TESTS = $(patsubst $(TESTDIR)/%.c,%,$(TEST_SRCS))
TEST_EXECUTABLES = $(addprefix target/tests/,$(TESTS))
# Exclude main.o when building test executables
TEST_OBJS = $(filter-out $(OBJDIR)/main.o,$(OBJS))

.PHONY: all clean tests

all: $(TARGET) tests

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	@mkdir -p $(@D)  # Create the obj directory if it doesn't exist
	$(CC) $(CFLAGS) -c -o $@ $<

tests: $(TEST_EXECUTABLES)

target/tests/%: $(TESTDIR)/%.c $(TEST_OBJS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET) $(OBJS) $(TEST_EXECUTABLES)

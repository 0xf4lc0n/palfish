CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Werror -O2 -fstack-protector-all -fPIE -fPIC -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Wformat-nonliteral -fno-strict-aliasing
TARGET = target/blowfish

SRCDIR = src
OBJDIR = obj

# List of source files
SRCS = $(wildcard $(SRCDIR)/*.c)

# Object files
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

# Dependencies
DEPS = $(wildcard $(SRCDIR)/*.h)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	@mkdir -p $(@D)  # Create the obj directory if it doesn't exist
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS)

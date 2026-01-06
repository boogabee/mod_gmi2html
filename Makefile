CC = gcc
APXS = apxs2
CFLAGS = -Wall -Wextra -fPIC
APACHE_INCLUDES = -I/usr/include/apache2 -I/usr/include/apr-1.0

# Source files
SOURCES = src/mod_gmi2html.c src/gemini_parser.c
OBJECTS = $(SOURCES:.c=.o)

# Default target
.PHONY: all install clean test

all: mod_gmi2html.so

# Compile object files
%.o: %.c
	$(CC) $(CFLAGS) $(APACHE_INCLUDES) -c $< -o $@

# Build Apache module
mod_gmi2html.so: $(OBJECTS)
	$(CC) $(CFLAGS) -shared $(OBJECTS) -o $@

# Install the module
install: mod_gmi2html.so
	$(APXS) -i -a -n gmi2html mod_gmi2html.so

# Install for development/testing (without enabling)
install-dev: mod_gmi2html.so
	$(APXS) -i -n gmi2html mod_gmi2html.so

# Clean build artifacts
clean:
	rm -f $(OBJECTS) mod_gmi2html.so *.o
	rm -f src/*.o src/*.so

# Test build (compile only)
test: clean all

.PHONY: all install clean test install-dev

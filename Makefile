# very simple makefile just to simplify the build
PYTHON_CFLAGS ?= `python-config --cflags`
PYTHON_LDFLAGS ?= `python-config --ldflags`

RLIB_FILE = $(shell (rustc --crate-type=rlib --crate-file-name "src/lib.rs" 2> /dev/null))
DYLIB_FILE = $(shell (rustc --crate-type=dylib --crate-file-name "src/lib.rs" 2> /dev/null))
RLIB = lib/$(RLIB_FILE)
DYLIB = lib/$(DYLIB_FILE)

all: rust

c: lib/libjinja2.so

build/jinja2.o: src/jinja2.c include/jinja2.h
	mkdir -p build
	gcc -c -Wall -fPIC $(PYTHON_CFLAGS) src/jinja2.c -o build/jinja2.o

lib/libjinja2.so: build/jinja2.o
	mkdir -p lib
	gcc -shared -Wl,-soname,libjinja2.so -o lib/libjinja2.so build/jinja2.o $(PYTHON_LDFLAGS)

rust: src/jinja2_c.rs $(DYLIB)

$(DYLIB):
	rustc -L lib --out-dir=lib src/lib.rs

src/jinja2_c.rs: lib/libjinja2.so
	# I have rust-bindgen executable up a folder so change this path if you dont
	../rust-bindgen/bindgen -l jinja2 -o src/jinja2_c.rs include/jinja2.h

clean:
	rm -rf build lib src/jinja2_c.rs

.PHONY:	clean all rust c
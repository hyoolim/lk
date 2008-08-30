## compiler
COMPILER ?= cc
COMPILER_FLAGS ?= -O2 -g -Wall -ansi -pedantic -I.

## linker
LINKER ?= cc
LINKER_FLAGS ?=

## platform-specific
OS := $(shell uname -s)
ifneq (,$(findstring $(OS),Darwin Linux))
LINKER_FLAGS +=-ldl
endif
ifneq (,$(findstring $(OS),Linux))
LINKER_FLAGS += -lm
endif

## install
PREFIX ?= /usr/local

## output files
sources = $(wildcard base/*.c) $(wildcard *.c)
objects = $(addprefix tmp/,$(patsubst %.c,%.o,$(sources)))
depends = $(patsubst %.o,%.d,$(objects)) tmp/exe/lk.d

## main targets
.PHONY: all clean install test uninstall
all: tmp/exe/lk
clean:
	rm -rf tmp
	rm -rf include
install: all
	mkdir -p $(PREFIX)/bin
	cp tmp/exe/lk $(PREFIX)/bin
	mkdir -p $(PREFIX)/lib/lk
	cp -R lib/* $(PREFIX)/lib/lk
test: all
	for test in `find test -name '*.lk'`; do tmp/exe/lk -l lib $$test; done;
uninstall:
	rm -rf $(PREFIX)/bin/lk
	rm -rf $(PREFIX)/lib/lk

## calculated dependencies
-include $(depends)

## make sure tmp folder for build output exists
tmp/init:
	mkdir -p tmp
	mkdir -p tmp/base
	mkdir -p tmp/exe
	mkdir -p include/lk/base
	cp *.h include/lk
	cp base/*.h include/lk/base
	touch tmp/init
$(objects) $(depends): tmp/init

## how to build everything else
tmp/%.o: %.c
	$(COMPILER) $(COMPILER_FLAGS) -c $< -o $@
tmp/%.d: %.c
	$(COMPILER) $(COMPILER_FLAGS) -MM -MT tmp/$(patsubst %.c,%.d,$<) -MT tmp/$(patsubst %.c,%.o,$<) -MF $@ $<
tmp/exe/lk.o: exe/lk.c
	$(COMPILER) $(COMPILER_FLAGS) -c exe/lk.c -o tmp/exe/lk.o -DPREFIX=\"$(PREFIX)\"
tmp/exe/lk: $(objects) tmp/exe/lk.o
	$(COMPILER) $(LINKER_FLAGS) -o tmp/exe/lk $(objects) tmp/exe/lk.o

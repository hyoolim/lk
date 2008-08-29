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

## output files
sources = $(wildcard base/*.c) $(wildcard *.c)
objects = $(addprefix tmp/,$(patsubst %.c,%.o,$(sources)))
depends = $(patsubst %.o,%.d,$(objects)) tmp/exe/lk_mini.d tmp/exe/lk.d

## main targets
.PHONY: all clean install test
all: tmp/exe/lk
clean:
	rm -rf tmp
install: all
	mkdir -p $(BINDIR)
	cp tmp/exe/lk_mini $(BINDIR)
	cp tmp/exe/lk $(BINDIR)
	mkdir -p $(LIBDIR)
	cp lib/* $(LIBDIR)
test: tmp/exe/lk_mini
	for test in `find test -name '*.lk'`; do tmp/exe/lk_mini lib/lk.lk $$test; done;

## calculated dependencies
-include $(depends)
-include tmp/conf.dat

## make sure tmp folder for build output exists
tmp/init:
	mkdir -p tmp
	mkdir -p tmp/base
	mkdir -p tmp/exe
	touch tmp/init
$(objects) $(depends): tmp/init

## how to build everything else
tmp/%.o: %.c
	$(COMPILER) $(COMPILER_FLAGS) -c $< -o $@
tmp/%.d: %.c
	$(COMPILER) $(COMPILER_FLAGS) -MM -MT tmp/$(patsubst %.c,%.d,$<) -MT tmp/$(patsubst %.c,%.o,$<) -MF $@ $<
tmp/exe/lk_mini: $(objects) tmp/exe/lk_mini.o
	$(COMPILER) $(LINKER_FLAGS) -o tmp/exe/lk_mini $(objects) tmp/exe/lk_mini.o
tmp/conf.dat: tmp/exe/lk_mini conf.lk
	if [ ! -e tmp/conf.dat ]; then tmp/exe/lk_mini lib/lk.lk conf.lk tmp/conf.dat; fi;
tmp/exe/lk.o: exe/lk.c tmp/conf.dat
	$(COMPILER) $(COMPILER_FLAGS) -c exe/lk.c -o tmp/exe/lk.o -DMF_PREFIX='$(PREFIX)' -DMF_BINDIR='$(BINDIR)' -DMF_LIBDIR='$(LIBDIR)'
tmp/exe/lk: tmp/exe/lk_mini tmp/exe/lk.o
	$(COMPILER) $(LINKER_FLAGS) -o tmp/exe/lk $(objects) tmp/exe/lk.o

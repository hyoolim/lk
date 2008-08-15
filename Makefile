## compiler
COMPILER ?= cc
COMPILER_FLAGS ?= -O2 -g -Wall -ansi -pedantic

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
sources = $(shell cat sources)
objects = $(addprefix tmp/,$(patsubst %.c,%.o,$(sources)))
depends = $(patsubst %.o,%.d,$(objects))

## main targets
.PHONY: all clean install test
all: tmp/lk
clean:
	rm -rf tmp
install: all
	mkdir -p $(BINDIR)
	cp tmp/lk_mini $(BINDIR)
	cp tmp/lk $(BINDIR)
	mkdir -p $(LIBDIR)
	cp lib/* $(LIBDIR)
test: tmp/lk_mini
	for test in `find test -name '*.lk'`; do tmp/lk_mini lib/lk.lk $$test; done;

## calculated dependencies
-include $(depends)
-include tmp/configData

## make sure tmp folder for build output exists
tmp/init:
	mkdir -p tmp
	touch tmp/init
$(objects) $(depends): tmp/init

## how to build everything else
tmp/%.o: %.c
	$(COMPILER) $(COMPILER_FLAGS) -c $< -o $@
tmp/%.d: %.c
	$(COMPILER) -MM -MT tmp/$(patsubst %.c,%.d,$<) -MT tmp/$(patsubst %.c,%.o,$<) -MF $@ $<
tmp/lk_mini: $(objects) tmp/lk_mini.o
	$(COMPILER) $(LINKER_FLAGS) -o tmp/lk_mini $(objects) tmp/lk_mini.o
tmp/configData: tmp/lk_mini config.lk
	if [ ! -e tmp/configData ]; then tmp/lk_mini lib/lk.lk config.lk tmp/configData; fi;
tmp/lk.o: lk.c tmp/configData
	$(COMPILER) $(COMPILER_FLAGS) -c lk.c -o tmp/lk.o -DMF_PREFIX='$(PREFIX)' -DMF_BINDIR='$(BINDIR)' -DMF_LIBDIR='$(LIBDIR)'
tmp/lk: tmp/lk_mini tmp/lk.o
	$(COMPILER) $(LINKER_FLAGS) -o tmp/lk $(objects) tmp/lk.o

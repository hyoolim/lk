## main config.
CC	   := gcc
RANLIB := ranlib
AR	   := ar rcu
SED    := sed
RM     := rm -f
RMDIR  := rmdir

## source files.
cflags   := -Wall -ansi -pedantic-errors -g -I../../..
dflags   := -M -I../../..
sources	 := $(wildcard *.c)
objects	 := $(addprefix +build/,$(patsubst %.c,%.o,$(sources)))
depends  := $(patsubst %.o,%.d,$(objects))
libname	 := +build/liblk-extex.so

## main targets.
.PHONY: lib dep clean
lib: dep $(libname)
dep: $(depends)
clean:
	$(RM) +build/*
	$(RMDIR) +build

## calc'd dep.
$(shell mkdir -p +build)
-include $(depends)

## how to compile various obj/exe files.
$(libname): $(objects)
	$(CC) $(cflags) -dynamic -bundle -undefined suppress -flat_namespace -o $@ $(objects)
+build/%.o: %.c
	$(CC) $(cflags) -c $< -o $@
+build/%.d: %.c
	@set -e; $(RM) $@; \
	$(CC) $(dflags) $< | \
	$(SED) 's,$*\.o[ :]*,+build/$*.o : ,g' > $@

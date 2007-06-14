## required programs
AR     = ar rcu
CP     = cp
MKDIR  = mkdir -p
RANLIB = ranlib
RM     = rm -f
RMDIR  = rmdir
SED    = sed

## package name and version
NAME  = lk
MAJOR = 0
MINOR = 9
PATCH = 0

## compiler options
BUILD   = _build
CFLAGS  = -O2 -g -Wall -ansi -pedantic
DFLAGS  = -M
LDFLAGS =
LIBS    =

## platform-specific options
OS = $(shell uname -s)
ifeq ($(OS),Linux)
LDFLAGS += -rdynamic
LIBS += -ldl
endif

## source and output files
sources = $(shell cat sources)
objects = $(addprefix $(BUILD)/,$(patsubst %.c,%.o,$(sources)))
depends = $(patsubst %.o,%.d,$(objects))

## main targets
.PHONY: all test install clean
all: $(BUILD)/lk
test: $(BUILD)/lk_mini
	@echo
	@echo '>>>> Testing <<<<'
	@echo '-----------------'
	for test in `find ./test -name '*.lk'`; \
	do $(BUILD)/lk_mini library/lk.lk $$test; done;
install: all
	@echo
	@echo '>>>> Installing <<<<'
	@echo '--------------------'
	$(MKDIR) $(BINDIR)
	$(CP) $(BUILD)/lk_mini $(BINDIR)
	$(CP) $(BUILD)/lk $(BINDIR)
	$(MKDIR) $(LIBDIR)
	$(CP) library/* $(LIBDIR)
clean:
	@echo
	@echo '>>>> Cleaning <<<<'
	@echo '------------------'
	$(RM) $(BUILD)/*
	$(RMDIR) $(BUILD)

## calc'd dep
-include $(depends)
-include $(BUILD)/config
-include $(BUILD)/init

## misc targets - how to compile various obj files
$(BUILD)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/%.d: %.c
	@set -e; $(RM) $@; \
	$(CC) $(DFLAGS) $< | \
	$(SED) 's,$*\.o[ :]*,$(BUILD)/$*.o : ,g' > $@
$(BUILD)/init:
	@echo
	@echo '>>>> Initializing <<<'
	@echo '---------------------'
	$(MKDIR) $(BUILD)
$(BUILD)/lk_mini: $(objects) $(BUILD)/lk_mini.o
	@echo
	@echo '>>>> Building (lk_mini) <<<<'
	@echo '----------------------------'
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(BUILD)/lk_mini \
	$(objects) $(BUILD)/lk_mini.o $(LIBS)
$(BUILD)/config: $(BUILD)/lk_mini ./config.lk
	@echo
	@echo '>>>> Configuring <<<<'
	@echo '---------------------'
	if [ ! -e $(BUILD)/config ]; \
    then $(BUILD)/lk_mini ./library/lk.lk ./config.lk; fi;
$(BUILD)/lk.o: lk.c $(BUILD)/config
	$(CC) $(CFLAGS) -c lk.c -o $(BUILD)/lk.o \
	-DMF_PREFIX='$(PREFIX)' -DMF_BINDIR='$(BINDIR)' \
	-DMF_LIBDIR='$(LIBDIR)'
$(BUILD)/lk: $(BUILD)/lk_mini $(BUILD)/lk.o
	@echo
	@echo '>>>> Building <<<<'
	@echo '--  --------------'
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(BUILD)/lk \
	$(objects) $(BUILD)/lk.o $(LIBS)

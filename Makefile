-include Mfcommon

## extra options - only for lk
CFLAGS += -Wall -ansi -pedantic
OPTBUILD = ./optional/$(BUILD)

## source and output files
sources = $(shell cat sources)
objects = $(addprefix $(BUILD)/,$(patsubst %.c,%.o,$(sources)))
depends = $(patsubst %.o,%.d,$(objects))

## main targets
.PHONY: all test install clean
all: $(BUILD)/lk
test: $(BUILD)/lkwol
	@echo
	@echo '>>>> Running tests <<<<'
	@echo '-----------------------'
	for test in `find ./test -name '*.lk'`; \
	do $(BUILD)/lkwol library/lk.lk $$test; done;
install: all
	@echo
	@echo '>>>> Installing <<<<'
	@echo '--------------------'
	$(MKDIR) $(BINDIR)
	$(CP) $(BUILD)/lkwoo $(BINDIR)
	$(CP) $(BUILD)/lkwol $(BINDIR)
	$(CP) $(BUILD)/lk $(BINDIR)
	$(MKDIR) $(LIBDIR)
	$(CP) library/* $(LIBDIR)
clean:
	@echo
	@echo '>>>> Cleaning <<<<'
	@echo '----------------------'
	$(RM) $(BUILD)/*
	$(RMDIR) $(BUILD)
	$(MAKE) -C ./optional clean

## calc'd dep
-include $(depends)
-include $(BUILD)/config
-include $(BUILD)/init

## misc targets - how to compile various obj files
$(BUILD)/init:
	@echo
	@echo '>>>> Initializing <<<'
	@echo '---------------------'
	$(MKDIR) $(BUILD)
	$(MKDIR) $(OPTBUILD)
$(BUILD)/lkwoo: $(objects) $(BUILD)/lkwoo.o
	@echo
	@echo '>>>> Building lkwoo (lk interpreter without optional) <<<<'
	@echo '---------------------------------------------------------------'
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(BUILD)/lkwoo \
	$(objects) $(BUILD)/lkwoo.o $(LIBS)
$(BUILD)/config: $(BUILD)/lkwoo ./config.lk
	@echo
	@echo '>>>> Configuring lk <<<<'
	@echo '---------------------'
	if [ ! -e $(BUILD)/config ]; \
    then $(BUILD)/lkwoo ./library/lk.lk ./config.lk; fi;
$(OPTBUILD)/objects: $(BUILD)/lkwoo ./optional/config.lk
	@echo
	@echo '>>>> Configuring optional components <<<<'
	@echo '-----------------------------------------'
	$(BUILD)/lkwoo ./library/lk.lk ./optional/config.lk \
	$(OPTBUILD) || rm $(OPTBUILD)/objects
$(BUILD)/lkwol: $(BUILD)/lkwoo $(BUILD)/lkwol.o $(OPTBUILD)/objects
	@echo
	@echo '>>>> Building optional components <<<<'
	@echo '--------------------------------------'
	$(MAKE) -C ./optional
	@echo
	@echo '>>>> Building lkwol (lk interpreter without library) <<<<'
	@echo '--------------------------------------------------------------'
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(BUILD)/lkwol \
	$(objects) $(BUILD)/lkwol.o \
	$(addprefix $(OPTBUILD)/,$(shell cat $(OPTBUILD)/objects)) \
	$(LIBS) $(shell cat $(OPTBUILD)/libs)
$(BUILD)/lk.o: lk.c $(BUILD)/config
	$(CC) $(CFLAGS) -c lk.c -o $(BUILD)/lk.o \
	-DMF_PREFIX='$(PREFIX)' -DMF_BINDIR='$(BINDIR)' \
	-DMF_LIBDIR='$(LIBDIR)'
$(BUILD)/lk: $(BUILD)/lkwol $(BUILD)/lk.o
	@echo
	@echo '>>>> Building lk (lk interpreter) <<<<'
	@echo '-------------------------------------------'
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(BUILD)/lk \
	$(objects) $(BUILD)/lk.o \
	$(addprefix $(OPTBUILD)/,$(shell cat $(OPTBUILD)/objects)) \
	$(LIBS) $(shell cat $(OPTBUILD)/libs)

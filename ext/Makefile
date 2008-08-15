-include ../Mfcommon

## source and output files
objects = $(addprefix $(BUILD)/,$(shell cat $(BUILD)/objects))
depends = $(patsubst %.o,%.d,$(objects))

## main targets
.PHONY: all clean
all: $(objects)
clean:
	$(RM) $(BUILD)/*
	$(RMDIR) $(BUILD)

## calc'd dep
-include $(depends)

##
$(BUILD)/optional.o: $(BUILD)/optional.c
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/optional.d: $(BUILD)/optional.c
	@set -e; $(RM) $@; \
	$(CC) $(DFLAGS) $< | \
	$(SED) 's,$*\.o[ :]*,$(BUILD)/$*.o : ,g' > $@
$(BUILD)/%.o: %.c
	$(CC) $(CFLAGS) $(shell cat $(BUILD)/$(patsubst %.c,%.cf,$<)) -c $< -o $@
$(BUILD)/%.d: %.c
	@set -e; $(RM) $@; \
	$(CC) $(DFLAGS) $(shell cat $(BUILD)/$(patsubst %.c,%.cf,$<)) $< | \
	$(SED) 's,$*\.o[ :]*,$(BUILD)/$*.o : ,g' > $@

PLATFORM ?= linux

include ../../tools/scripts/generic_variables.mk

include ../../tools/scripts/examples.mk

include src.mk

include ../../tools/scripts/generic.mk

# Linux platform specific targets
ifeq ($(PLATFORM),linux)
# Build executable
adf4377_test: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

# Clean target
clean-linux:
	rm -f adf4377_test $(OBJS)

.PHONY: adf4377_test clean-linux
endif




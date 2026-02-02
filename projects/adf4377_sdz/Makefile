EXAMPLE ?= vendor_id
PLATFORM ?= linux

include ../../tools/scripts/generic_variables.mk

include ../../tools/scripts/examples.mk

include src.mk

include ../../tools/scripts/generic.mk

# Linux platform specific targets
ifeq ($(PLATFORM),linux)
# Build executable
adf4377: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

# Build shared library (excluding main.c and example files)
# Create PIC objects for shared library
PIC_OBJS = $(patsubst %.o,%.pic.o,$(filter-out src/platform/linux/main.o src/examples/basic/basic_example.o src/examples/vendor_id_example.o src/common/common_data.o, $(OBJS)))

$(OBJECTS_DIR)/%.pic.o: $$(call get_full_path, %).c | $$(@D)/.
	$(call print,[CC-PIC] $(notdir $<))
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

libadf4377.so: $(PIC_OBJS)
	$(CC) -shared $(PIC_OBJS) -o $@ $(LDFLAGS)

# Build static library (including platform operations)
STATIC_OBJS = $(filter-out src/platform/linux/main.o src/examples/basic/basic_example.o src/examples/vendor_id_example.o src/common/common_data.o, $(OBJS))

libadf4377.a: $(STATIC_OBJS)
	ar rcs $@ $(STATIC_OBJS)

# Clean target
clean-linux:
	rm -f adf4377 libadf4377.so libadf4377.a $(OBJS) $(PIC_OBJS)

.PHONY: adf4377 libadf4377.so libadf4377.a clean-linux
endif
PLATFORM ?= linux

include ../../tools/scripts/generic_variables.mk

include ../../tools/scripts/examples.mk

include src.mk

include ../../tools/scripts/generic.mk

# Linux platform specific targets
ifeq ($(PLATFORM),linux)
# Build executable (MSB first)
adf4377_test: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

# LSB-first executable: same sources but common_data.c built with ADF4377_TEST_LSB_FIRST
COMMON_DATA_OBJ   = $(filter %/common_data.o,$(OBJS))
COMMON_DATA_LSB_O = $(patsubst %common_data.o,%common_data_lsb.o,$(COMMON_DATA_OBJ))

adf4377_test_lsb: update pre_build $(filter-out %/common_data.o,$(OBJS)) $(COMMON_DATA_LSB_O)
	$(call print,[LD] adf4377_test_lsb)
	$(CC) $(CFLAGS) $(filter-out %/common_data.o,$(OBJS)) $(COMMON_DATA_LSB_O) -o $@ $(LDFLAGS)

$(COMMON_DATA_LSB_O): $(PROJECT)/src/common/common_data.c pre_build | $$(@D)/.
	$(call print,[CC] common_data.c (LSB first))
	$(CC) -c @$(CFLAGS_FILE) -DADF4377_TEST_LSB_FIRST $< -o $@

# Clean target
clean-linux:
	rm -f adf4377_test adf4377_test_lsb $(COMMON_DATA_LSB_O) $(OBJS)

.PHONY: adf4377_test adf4377_test_lsb clean-linux
endif




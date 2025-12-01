COMMON_DIR := common
kalertd_SRCS    := kalertd.c $(COMMON_DIR)/kalert_event.c
sub_test_SRCS   := sub_test.c

TARGETS := kalertd sub_test

BINS := $(addprefix $(BUILD_DIR)/,$(TARGETS))

CFLAGS   := -I$(SRC_ROOT)/include -Wall -O2
LDFLAGS  := -L$(LIB_BUILD) -lkalert -lev

.PHONY: all clean

all: $(BINS)

$(BUILD_DIR)/%:
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $($(*F)_SRCS) -o $@ $(LDFLAGS)

clean:
	rm -f $(BINS)

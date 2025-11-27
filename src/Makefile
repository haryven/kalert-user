SRCS     := $(wildcard *.c)
BINS     := $(patsubst %.c,$(BUILD_DIR)/%,$(SRCS))

CFLAGS   := -I$(SRC_ROOT)/include -Wall -O2
LDFLAGS  := -L$(LIB_BUILD) -lkalert -lev

.PHONY: all clean

all: $(BINS)

$(BUILD_DIR)/%: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(BINS)


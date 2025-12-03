# Common source directory
COMMON_DIR := common

# Configuration area - only modify here when adding new targets
TARGETS := kalertd sub_test

# Source file definitions for each target
kalertd_SRCS := \
		kalertd.c \
		$(COMMON_DIR)/kalert_event.c \
		$(COMMON_DIR)/common.c
sub_test_SRCS := sub_test.c

# Compiler and linker flags
CFLAGS  := -I$(SRC_ROOT)/include -Wall -O2 -MMD -MP
LDFLAGS := -L$(LIB_BUILD) -lkalert -lev

# Object files go to hidden .obj directory, binaries stay in BUILD_DIR root
OBJ_DIR := $(BUILD_DIR)/.obj
BINS := $(addprefix $(BUILD_DIR)/, $(TARGETS))
OBJS := $(foreach target,$(TARGETS),\
        $(addprefix $(OBJ_DIR)/, $(notdir $($(target)_SRCS:.c=.o))))

# Source directories to search for .c files
SOURCE_DIRS := . $(COMMON_DIR)

.PHONY: all clean
all: $(BINS)

# Dynamically generate linking rules for each target
$(foreach target,$(TARGETS),\
  $(eval $(BUILD_DIR)/$(target): $(addprefix $(OBJ_DIR)/, $(notdir $($(target)_SRCS:.c=.o))) ; \
  mkdir -p $(OBJ_DIR) && $(CC) $(CFLAGS) $$^ $(LDFLAGS) -o $$@))

# Use vpath to handle source files from different directories
vpath %.c $(SOURCE_DIRS)

# Compilation rule: all .o files go to .obj directory
$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Create necessary directories
$(BUILD_DIR) $(OBJ_DIR):
	@mkdir -p $@

# Include automatically generated dependency files
-include $(OBJS:.o=.d)

clean:
	rm -rf $(BUILD_DIR)

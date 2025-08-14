LIB_NAME     := libkalert
LIB_MAJOR    := 1
LIB_MINOR    := 0
LIB_PATCH    := 0

LIB_SO       := $(LIB_NAME).so.$(LIB_MAJOR).$(LIB_MINOR).$(LIB_PATCH)
LIB_SONAME   := $(LIB_NAME).so.$(LIB_MAJOR)
LIB_A        := $(LIB_NAME).a

SRC          := $(wildcard *.c)
OBJ          := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC))

CFLAGS       := -Wall -Wextra -O2 -fPIC -I$(SRC_ROOT)/include
LDFLAGS      := -shared -Wl,-soname,$(LIB_SONAME)

.PHONY: all clean

all: $(BUILD_DIR)/$(LIB_SO) $(BUILD_DIR)/$(LIB_A)
	@find $(BUILD_DIR) -name '*.o' -delete

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/$(LIB_SO): $(OBJ)
	mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) -o $@ $^
	ln -sf $(LIB_SO) $(BUILD_DIR)/$(LIB_SONAME)
	ln -sf $(LIB_SONAME) $(BUILD_DIR)/$(LIB_NAME).so

$(BUILD_DIR)/$(LIB_A): $(OBJ)
	mkdir -p $(dir $@)
	ar rcs $@ $^

clean:
	rm -f $(OBJ) $(BUILD_DIR)/$(LIB_SO) $(BUILD_DIR)/$(LIB_SONAME) $(BUILD_DIR)/$(LIB_NAME).so $(BUILD_DIR)/$(LIB_A)


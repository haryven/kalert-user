SRC_ROOT    := $(shell pwd)
LIB_SRC     := $(SRC_ROOT)/libkalert
APP_SRC     := $(SRC_ROOT)/src
BUILD_DIR   := $(SRC_ROOT)/build
LIB_BUILD   := $(BUILD_DIR)/libkalert
APP_BUILD   := $(BUILD_DIR)/app

PREFIX      ?= /usr
DESTDIR     ?=

LIB_NAME    := libkalert

export SRC_ROOT BUILD_DIR LIB_BUILD APP_BUILD PREFIX DESTDIR LIB_NAME

.PHONY: all lib app clean install uninstall

all: lib app

lib:
	$(MAKE) -C $(LIB_SRC) BUILD_DIR=$(LIB_BUILD)

app: lib
	$(MAKE) -C $(APP_SRC) BUILD_DIR=$(APP_BUILD)

clean:
	$(MAKE) -C $(LIB_SRC) clean BUILD_DIR=$(LIB_BUILD)
	$(MAKE) -C $(APP_SRC) clean BUILD_DIR=$(APP_BUILD)
	rm -rf $(BUILD_DIR)

install: all
	@echo "Installing library files..."
	mkdir -p $(DESTDIR)$(PREFIX)/lib $(DESTDIR)$(PREFIX)/include/$(LIB_NAME)
	cp -a $(LIB_BUILD)/*.so* $(DESTDIR)$(PREFIX)/lib/ || true
	cp -a $(LIB_BUILD)/*.a   $(DESTDIR)$(PREFIX)/lib/ || true
	cp -a $(SRC_ROOT)/include/$(LIB_NAME)/* $(DESTDIR)$(PREFIX)/include/$(LIB_NAME)/

	@echo "Installing binaries..."
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -a $(APP_BUILD)/* $(DESTDIR)$(PREFIX)/bin/

uninstall:
	@echo "Removing installed files..."
	# remove head file
	for f in $(notdir $(wildcard $(LIB_BUILD)/*)); do \
	    rm -f $(DESTDIR)$(PREFIX)/lib/$$f; \
	done
	# remove lib
	rm -rf $(DESTDIR)$(PREFIX)/include/$(LIB_NAME)
	# remove app
	for f in $(notdir $(wildcard $(APP_BUILD)/*)); do \
	    rm -f $(DESTDIR)$(PREFIX)/bin/$$f; \
	done


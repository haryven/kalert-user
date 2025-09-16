SRC_ROOT    := $(shell pwd)
LIB_SRC     := $(SRC_ROOT)/libkalert
APP_SRC     := $(SRC_ROOT)/src
BUILD_DIR   := $(SRC_ROOT)/build
LIB_BUILD   := $(BUILD_DIR)/libkalert
APP_BUILD   := $(BUILD_DIR)/app

DESTDIR		?=
PREFIX		?= /usr/local
libdir		?= $(PREFIX)/lib
bindir		?= $(PREFIX)/bin


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
	mkdir -p $(DESTDIR)$(libdir)/ $(DESTDIR)$(PREFIX)/include/$(LIB_NAME)
	cp -a $(LIB_BUILD)/*.so* $(DESTDIR)$(libdir) || true
	cp -a $(LIB_BUILD)/*.a   $(DESTDIR)$(libdir) || true
	cp -a $(SRC_ROOT)/include/$(LIB_NAME)/* $(DESTDIR)$(PREFIX)/include/$(LIB_NAME)/

	@echo "Installing binaries..."
	mkdir -p $(DESTDIR)$(bindir)
	cp -a $(APP_BUILD)/* $(DESTDIR)$(bindir)

uninstall:
	@echo "Removing installed files..."
	# remove libs
	for f in $(notdir $(wildcard $(LIB_BUILD)/*)); do \
	    rm -f $(DESTDIR)$(libdir)/$$f; \
	done
	# remove headers
	rm -rf $(DESTDIR)$(PREFIX)/include/$(LIB_NAME)
	# remove apps
	for f in $(notdir $(wildcard $(APP_BUILD)/*)); do \
	    rm -f $(DESTDIR)$(bindir)/$$f; \
	done

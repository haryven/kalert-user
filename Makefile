SRC_ROOT    := $(shell pwd)
LIB_SRC     := $(SRC_ROOT)/libkalert
APP_SRC     := $(SRC_ROOT)/src
BUILD_DIR   := $(SRC_ROOT)/build
LIB_BUILD   := $(BUILD_DIR)/libkalert
APP_BUILD   := $(BUILD_DIR)/app

DESTDIR         ?=
PREFIX          ?= /usr/local
libdir          ?= $(PREFIX)/lib
bindir          ?= $(PREFIX)/bin
includedir      ?= $(PREFIX)/include

INSTALL          ?= install
INSTALL_DIR      = $(INSTALL) -d
INSTALL_FILE     = $(INSTALL) -m 0644
INSTALL_BIN      = $(INSTALL) -m 0755

LIB_NAME    := libkalert
VERSION := 1.0.0
PACKAGE := kalert-user-$(VERSION)
DIST_FILES := $(shell git ls-files)

export SRC_ROOT BUILD_DIR LIB_BUILD APP_BUILD PREFIX DESTDIR LIB_NAME

.PHONY: all lib app clean install uninstall dist

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

	$(INSTALL_DIR) $(DESTDIR)$(libdir)
	$(INSTALL_DIR) $(DESTDIR)$(includedir)/$(LIB_NAME)

	# Install libraries
	$(INSTALL_FILE) $(LIB_BUILD)/*.so* $(DESTDIR)$(libdir) 2>/dev/null || true
	$(INSTALL_FILE) $(LIB_BUILD)/*.a   $(DESTDIR)$(libdir) 2>/dev/null || true

	# Install headers
	$(INSTALL_FILE) $(SRC_ROOT)/include/$(LIB_NAME)/* $(DESTDIR)$(includedir)/$(LIB_NAME)

	@echo "Installing binaries..."
	$(INSTALL_DIR) $(DESTDIR)$(bindir)
	$(INSTALL_BIN) $(APP_BUILD)/* $(DESTDIR)$(bindir)

uninstall:
	@echo "Removing installed files..."

	# Remove libraries
	for f in $(notdir $(wildcard $(LIB_BUILD)/*)); do \
	    rm -f $(DESTDIR)$(libdir)/$$f; \
	done

	# Remove headers
	rm -rf $(DESTDIR)$(includedir)/$(LIB_NAME)

	# Remove binaries
	for f in $(notdir $(wildcard $(APP_BUILD)/*)); do \
	    rm -f $(DESTDIR)$(bindir)/$$f; \
	done

dist:
	@echo "Creating $(PACKAGE).tar.gz ..."
	tar -czf $(PACKAGE).tar.gz \
		--transform 's,^,$(PACKAGE)/,' \
		$(DIST_FILES)
	@echo "Done."

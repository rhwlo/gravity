ifneq (,$(wildcard vars.mak))
-include vars.mak
endif

OSFAMILY	:= $(shell (uname -s))
MAKE_DIR	:= $(PWD)
SERIAL_DEV	?= unknown
FQBN		?= arduino:avr:uno

BUILD_DIR	:= $(subst :,.,build/$(FQBN))
SRCINO		:= $(wildcard *.ino)
SRC		:= $(wildcard *.ino *.cpp) $(shell find src -name '*.cpp')
HDRS		:= $(wildcard *.h) $(shell find src -name '*.h')
HEX		:= $(BUILD_DIR)/$(SRCINO).hex
ELF		:= $(BUILD_DIR)/$(SRCINO).elf
AVRDUDE_FLAGS 	:= -B 125kHz -v -patmega328p -cstk500v1 -b19200

all: $(ELF) upload
.PHONY: all

compile: $(ELF)
.PHONY: compile

$(ELF): $(SRC) $(HDRS)
	echo "Building $(SRCINO)"
	arduino-cli compile -b $(FQBN) --build-path $(BUILD_DIR) $(VFLAG) $(CUSTOM_LIBS)

upload: compile
ifeq ($(SERIAL_DEV),unknown)
	$(error No serial device is defined)
endif
	@echo "Uploading sketch\n"; \
	avrdude $(AVRDUDE_FLAGS) -P$(SERIAL_DEV) -Uflash:w:$(HEX):i

clean:
	@echo "Cleaning the build directory"
	rm -rf $(BUILD_DIR)
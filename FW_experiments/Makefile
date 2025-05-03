# We only allow compilation on linux!
ifneq ($(shell uname), Linux)
$(error OS must be Linux!)
endif

TARGET 		  = main
CODE_LOCATION = 0x6000
DATA_LOCATION = 0x6000

# Check if all required tools are on the system.
REQUIRED = sdcc sdar sdasz80
K := $(foreach exec,$(REQUIRED),\
    $(if $(shell which $(exec)),,$(error "$(exec) not found. Please install or add to path.")))

# Global settings: folders.
ROOT        = $(realpath .)
BUILD_DIR   = $(ROOT)/build
SRC_DIR     = $(ROOT)/src

# Source files
SRC         = $(wildcard $(SRC_DIR)/*.c)
ASM_SRC     = $(SRC_DIR)/crt0.s

# Object files
REL_FILES   = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.rel, $(SRC))
ASM_OBJ     = $(patsubst $(SRC_DIR)/%.s, $(BUILD_DIR)/%.rel, $(ASM_SRC))
OBJECTS     = $(ASM_OBJ) $(REL_FILES)

# Global settings: tools.
CC          = sdcc
AS          = sdasz80
CFLAGS      = --std-c11 -mz80 --debug --nostdinc --no-std-crt0 --nostdinc --nostdlib
ASFLAGS     = -xlos -g
LDFLAGS     = -mz80 -Wl -y --code-loc $(CODE_LOCATION) --data-loc $(DATA_LOCATION)

.PHONY: all
all: $(BUILD_DIR)/main.bin

# Create binary from IHX
$(BUILD_DIR)/main.bin: $(BUILD_DIR)/main.ihx
	sdobjcopy -I ihex -O binary $< $@

# Link all objects
$(BUILD_DIR)/main.ihx: $(OBJECTS)
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) -Llib -llibsdcc-z80 $^

# Compile C sources
$(BUILD_DIR)/%.rel: $(SRC_DIR)/%.c
	$(CC) -o $@ -c $(CFLAGS) $<

# Assemble ASM sources
$(BUILD_DIR)/%.rel: $(SRC_DIR)/%.s
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

.PHONY: font
font:
	rm -f $(SRC_DIR)/font.h
	touch $(SRC_DIR)/font.h
	echo -n "const " >> $(SRC_DIR)/font.h
	xxd -i $(SRC_DIR)/font_en.ch8 >> $(SRC_DIR)/font.h

.PHONY: flash
flash:
	PCProg.exe COM18 $(BUILD_DIR)/main.bin
# We only allow compilation on linux!
ifneq ($(shell uname), Linux)
$(error OS must be Linux!)
endif

TARGET        = main
CODE_LOCATION = 0x0000
DATA_LOCATION = 0xa000

# Check if all required tools are on the system.
REQUIRED = sdcc sdar sdasz80
K := $(foreach exec,$(REQUIRED),\
    $(if $(shell which $(exec)),,$(error "$(exec) not found. Please install or add to path.")))

# Global settings: folders.
ROOT        = $(realpath .)
BUILD_DIR   = $(ROOT)/build
SRC_DIR     = $(ROOT)/src
ASSETS_DIR  = $(ROOT)/assets

# Source files
SRC         = $(wildcard $(SRC_DIR)/*.c)
ASM_SRC     = $(SRC_DIR)/crt0.s
FONT_SRC    = $(ASSETS_DIR)/font_en.ch8

# Object files
REL_FILES   = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.rel, $(SRC))
ASM_OBJ     = $(patsubst $(SRC_DIR)/%.s, $(BUILD_DIR)/%.rel, $(ASM_SRC))
OBJECTS     = $(ASM_OBJ) $(REL_FILES)

# Global settings: tools.
CC          = sdcc
AS          = sdasz80
CFLAGS      = --std-c11 -mz80 --debug --nostdinc --no-std-crt0 --nostdlib
ASFLAGS     = -xlos -g
LDFLAGS     = -mz80 -Wl -y --code-loc $(CODE_LOCATION) --data-loc $(DATA_LOCATION)

CFLAGS      += -DBAREMETAL

.PHONY: all
all: $(BUILD_DIR)/main.bin

# Create binary from IHX
$(BUILD_DIR)/main.bin: $(BUILD_DIR)/main.ihx
	sdobjcopy -I ihex -O binary $< $@

# Link all objects
$(BUILD_DIR)/main.ihx: $(OBJECTS)
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) -Llib -llibsdcc-z80 $^

# Compile C sources (with font.h dependency)
$(BUILD_DIR)/%.rel: $(SRC_DIR)/%.c $(SRC_DIR)/font.h
	@mkdir -p $(@D)
	$(CC) -o $@ -c $(CFLAGS) $<

# Assemble ASM sources
$(BUILD_DIR)/%.rel: $(SRC_DIR)/%.s
	@mkdir -p $(@D)
	$(AS) $(ASFLAGS) -o $@ $<

# Generate font.h from font file
$(SRC_DIR)/font.h: $(FONT_SRC)
	@echo "Generating font.h"
	@echo "const unsigned char font_data[] = {" > $@
	@hexdump -v -e '8/1 "0x%02x, " "\n"' $< | sed 's/0x  ,//g' >> $@
	@echo "};" >> $@
# @echo "const unsigned int font_data_len = sizeof(font_data);" >> $@

clean:
	rm -rf $(BUILD_DIR)

flash:
	PCProg.exe COM18 build/main.bin
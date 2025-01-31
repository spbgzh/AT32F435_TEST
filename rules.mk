# 'make V=1' will show all compiler calls.
V	?= 0
ifeq ($(V),0)
Q	:= @
NULL	:= 2>/dev/null
endif

PREFIX		?=	$(ARM_TOOCHAIN)/arm-none-eabi-
CC			=	$(PREFIX)gcc
AS			=	$(PREFIX)as
LD			=	$(PREFIX)ld
OBJCOPY		=	$(PREFIX)objcopy
# `$(shell pwd)` or `.`, both works
TOP			=	$(shell pwd)
BDIR		=	$(TOP)/$(BUILD_DIR)

# For each direcotry, add it to CSRCS
CSRCS	:=	$(foreach dir, $(CDIRS), $(shell find $(TOP)/$(dir) -maxdepth 1 -name '*.c'))
# Add single c source files to CSRCS
CSRCS	+=	$(addprefix $(TOP)/, $(CFILES))
# Then assembly source folders and files
ASRCS	:=	$(foreach dir, $(ADIRS), $(shell find $(TOP)/$(dir) -maxdepth 1 -name '*.s'))
ASRCS	+=	$(addprefix $(TOP)/, $(AFILES))

# Fill object files with c and asm files (keep source directory structure)
OBJS	=	$(CSRCS:$(TOP)/%.c=$(BDIR)/%.o)
OBJS	+=	$(ASRCS:$(TOP)/%.s=$(BDIR)/%.o)
# d files for detecting h file changes
DEPS	=	$(CSRCS:$(TOP)/%.c=$(BDIR)/%.d)

# Arch and target specified flags
ARCH_FLAGS	:=	-mthumb	-mcpu=cortex-m4	-mfloat-abi=hard -mfpu=fpv4-sp-d16
# Debug options, -gdwarf-2 for debug, -g0 for release 
# https://gcc.gnu.org/onlinedocs/gcc-12.2.0/gcc/Debugging-Options.html
#  -g: system’s native format, -g0:off, -g/g1,-g2,-g3 -> more verbosely
#  -ggdb: for gdb, -ggdb0:off, -ggdb/ggdb1,-ggdb2,-ggdb3 -> more verbosely
#  -gdwarf: in DWARF format, -gdwarf-2,-gdwarf-3,-gdwarf-4,-gdwarf-5
DEBUG_FLAGS	?=	-gdwarf-3

# c flags
OPT			?=	-O3
CSTD		?=	-std=c99
CFLAGS		+=	$(ARCH_FLAGS) $(DEBUG_FLAGS) $(OPT) $(CSTD) $(addprefix -D, $(LIB_FLAGS)) -Wall -ffunction-sections -fdata-sections

# asm flags
AFLAGS	+=	$(ARCH_FLAGS) $(DEBUG_FLAGS) $(OPT) -Wa,--warn

# ld flags
TGT_LDFLAGS	+=	$(ARCH_FLAGS) -specs=nano.specs -specs=nosys.specs -static -lc -lm \
				-Wl,-Map=$(BDIR)/$(PROJECT).map \
				-Wl,--gc-sections \
				-Wl,--print-memory-usage \
				-Wl,--no-warn-rwx-segments


# include paths
TGT_INCFLAGS	:=	$(addprefix -I $(TOP)/, $(INCLUDES))


.PHONY: all clean flash echo

all: $(BDIR)/$(PROJECT).elf $(BDIR)/$(PROJECT).bin $(BDIR)/$(PROJECT).hex

# for debug
echo:
	$(info 1. $(AFILES))
	$(info 2. $(ASRCS))
	$(info 3. $(CSRCS))
	$(info 4. $(OBJS))
	$(info 5. $(TGT_INCFLAGS))

# include d files without non-exist warning
-include $(DEPS)

# Compile c to obj -- should be `$(BDIR)/%.o: $(TOP)/%.c`, but since $(TOP) is base folder so non-path also works
$(BDIR)/%.o: %.c
	@printf "  CC\t$<\n"
	@mkdir -p $(dir $@)
	$(Q)$(CC) $(CFLAGS) $(TGT_INCFLAGS) -o $@ -c $< -MD -MF $(BDIR)/$*.d -MP

$(BDIR)/%.o: %.s
	@printf "  AS\t$<\n"
	@mkdir -p $(dir $@)
	$(Q)$(CC) $(AFLAGS) -o $@ -c $<

$(BDIR)/$(PROJECT).elf: $(OBJS) $(TOP)/$(LDSCRIPT)
	@printf "  LD\t$@\n"
	$(Q)$(CC) $(TGT_LDFLAGS) -T$(TOP)/$(LDSCRIPT) $(OBJS) -o $@

%.bin: %.elf
	@printf "  OBJCP\t$@\n"
	$(Q)$(OBJCOPY) -O binary  $< $@

%.hex: %.elf
	@printf "  OBJCP\t$@\n"
	$(Q)$(OBJCOPY) -O ihex  $< $@

clean:
	rm -rf $(BDIR)/*

flash:
	$(JLINKEXE) -device $(JLINK_DEVICE) -if swd -speed 4000 -CommanderScript $(TOP)/Misc/flash.jlink
#	$(JLINKEXE) -device $(JLINK_EXT_DEVICE) -if swd -speed 4000 -CommanderScript $(TOP)/Misc/flashExt.jlink

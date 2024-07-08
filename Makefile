##### Project #####

PROJECT		?=	App
BUILD_DIR	=	Build

# Programmer, jlink or pyocd
FLASH_PROGRM	?=	jlink

##### Toolchains #######

ARM_TOOCHAIN		?=	/Applications/ArmGNUToolchain/12.2.rel1/arm-none-eabi/bin

# path to JLinkExe
JLINKEXE			?=	/Applications/SEGGER/JLink_V794e/JLinkExe
JLINK_DEVICE		?=	AT32F435RMT7

#JLINK_EXT_DEVICE	?=	qspi_algorithm_w25q128

##### Paths ############

# Link descript file for this chip
LDSCRIPT		=	Libraries/cmsis/cm4/device_support/startup/gcc/linker/AT32F435xM_FLASH.ld
# Library build flags
LIB_FLAGS		=	USE_STDPERIPH_DRIVER	AT32F435RMT7

# C source folders
CDIRS	:=	User \
		User/src \
		Libraries/cmsis/cm4/device_support \
		Libraries/drivers/src
		
# C source files (if there are any single ones)
CFILES	:=

# ASM source folders
ADIRS	:= User/src
# ASM single files
AFILES	:= Libraries/cmsis/cm4/device_support/startup/gcc/startup_at32f435_437.s

# Include paths
INCLUDES	:=	User \
			User/inc \
			Libraries/cmsis/cm4/core_support \
			Libraries/cmsis/cm4/device_support \
			Libraries/drivers/inc

include	./rules.mk

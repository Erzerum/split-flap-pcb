TARGET = code

DEBUG = 1
OPT = -Og -O0

BUILD = build



RED=$(shell tput setaf 1)
GREEN=$(shell tput setaf 2)
NORMAL=$(shell tput sgr0)

######################################
# source
######################################
# C sources

C_SOURCES   = $(shell find Core Drivers src -name '*.c' 2>/dev/null)
CXX_SOURCES   = $(shell find Core Drivers src -name '*.cpp' 2>/dev/null)
ASM_SOURCES = $(shell find gen -name '*.s' 2>/dev/null)

#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
C++ = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
C++ = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m0
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# C defines
C_DEFS = -DUSE_HAL_DRIVER -DSTM32F030x8

# AS includes
AS_INCLUDES = 

# C includes
C_INCLUDES =  \
-I./Core/Inc \
-I./Drivers/STM32F0xx_HAL_Driver/Inc \
-I./Drivers/STM32F0xx_HAL_Driver/Inc/Legacy \
-I./Drivers/CMSIS/Device/ST/STM32F0xx/Include \
-I./Drivers/CMSIS/Include

WARNING_FLAGS = -Wall -Werror -Wno-error=unused-variable -Wno-error=unused-but-set-variable -Wno-error=unused-function

# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) \
		  -Wall -fdata-sections -ffunction-sections

CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) \
		  -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = gen/STM32F030C8Tx_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS)\
		  -Wl,-Map=$(BUILD)/$(TARGET).map,--cref -Wl,--gc-sections\
		  -Wl,--no-warn-rwx-segments

# default action: build all
all: $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).hex $(BUILD)/$(TARGET).bin


S ?= @

#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))
# C++ objects
OBJECTS += $(addprefix $(BUILD)/cpp/,$(notdir $(CXX_SOURCES:.cpp=.o)))
vpath %.cpp $(sort $(dir $(CXX_SOURCES)))

$(BUILD)/cpp/%.o: %.cpp Makefile | $(BUILD) 
	@echo "$(GREEN)[ compiling ]$(NORMAL) " $<
	$(S)$(C++) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD)/$(notdir $(<:.c=.lst)) $< -o $@ $(WARNING_FLAGS)
$(BUILD)/%.o: %.c Makefile | $(BUILD) 
	@echo "$(GREEN)[ compiling ]$(NORMAL) " $<
	$(S)$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD)/$(notdir $(<:.c=.lst)) $< -o $@
$(BUILD)/%.o: %.s Makefile | $(BUILD)
	@echo "$(GREEN)[ compiling ]$(NORMAL) " $<
	$(S)$(AS) -c $(CFLAGS) $< -o $@
$(BUILD)/%.o: %.S Makefile | $(BUILD)
	@echo "$(GREEN)[ compiling ]$(NORMAL) " $<
	$(S)$(AS) -c $(CFLAGS) $< -o $@

$(BUILD)/$(TARGET).elf: $(OBJECTS) Makefile
	@echo "$(GREEN)[ linking   ]$(NORMAL) " $@
	$(S)$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(S)$(SZ) $@

$(BUILD)/%.hex: $(BUILD)/%.elf | $(BUILD)
	$(S)$(HEX) $< $@
	
$(BUILD)/%.bin: $(BUILD)/%.elf | $(BUILD)
	$(S)$(BIN) $< $@	
	
$(BUILD):
	mkdir $@		
	mkdir $@/cpp/

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD)

cleanall:
	-rm -fR $(BUILD) Core Drivers gen
  
bear:
	make -f Makefile clean
	bear -- make -f Makefile -j6
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD)/*.d)

# *** EOF ***

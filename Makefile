PROJECT=Dust_Monitor_Ras
# Set the FLAG OS_SYSTEM - LINUX or BEAGLEBONEBLACK it depends which sistem are you use to build. 
OS_SYSTEM=LINUX

SYSROOT_PATH=/home/muxamor/Develop/RaspberryPi/sysroot

# Directory for C-Source
vpath %.c $(CURDIR)/source
vpath %.c $(CURDIR)/source/OLED
vpath %.c $(CURDIR)/source/OLED/Fonts

# Directory for includes
CINCLUDE = -I$(CURDIR)/include\
		   -I$(CURDIR)/include/OLED\
		   -I$(CURDIR)/include/OLED/Fonts\
		   -I/home/muxamor/Develop/RaspberryPi/sysroot/usr/include
			
# Directory for object files
OBJDIR = $(CURDIR)/object

# Other dependencies
DEPS = # \
 Makefile \
 include/main.h\
 include/fonts.h\
 include/OLED_GUI.h\
 include/OLED_Driver.h\
 include/DEV_Config.h

# Compiler object files 
COBJ =\
 $(OBJDIR)/main.o\
 $(OBJDIR)/font8.o\
 $(OBJDIR)/font12.o\
 $(OBJDIR)/font16.o\
 $(OBJDIR)/font20.o\
 $(OBJDIR)/font24.o\
 $(OBJDIR)/OLED_GUI.o\
 $(OBJDIR)/OLED_Driver.o\
 $(OBJDIR)/DEV_Config.o
 
# gcc binaries to use. Write correct path on your OS system 
ifeq ($(OS_SYSTEM),LINUX)
CC = "/opt/cross-pi-gcc-6.3.0-2/bin/arm-linux-gnueabihf-gcc"
LD = "/opt/cross-pi-gcc-6.3.0-2/bin/arm-linux-gnueabihf-gcc"
else ifeq ($(OS_SYSTEM),RASSBERYPI)
CC = "gcc"
LD = "gcc"
endif

REMOVE = rm -f

# Compiler options
CFLAGS = -marm
CFLAGS += -march=armv8-a
CFLAGS += -mfloat-abi=hard
CFLAGS += -mtune=cortex-a53
CFLAGS += -mfpu=crypto-neon-fp-armv8
CFLAGS += -O0 
CFLAGS += -g 
CFLAGS += -I.
CFLAGS += $(CINCLUDE)
CFLAGS += $(CDEFINE)

LDFLAGS = --sysroot=$(SYSROOT_PATH)

LIB = -lwiringPi

# for a better output
MSG_EMPTYLINE = . 
MSG_COMPILING = ---COMPILE--- 
MSG_LINKING = ---LINK--- 
MSG_SUCCESS = ---SUCCESS---

#Test toolchain
test: 
	$(CC) --version
	$(MAKE) --version

# Our favourite
all: $(PROJECT)

# Linker call
$(PROJECT): $(COBJ)
	@echo $(MSG_EMPTYLINE)
	@echo $(MSG_LINKING)
	$(LD) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LIB)
	@echo $(MSG_EMPTYLINE)
	@echo $(MSG_SUCCESS) $(PROJECT)

# Compiler call
$(COBJ): $(OBJDIR)/%.o: %.c $(DEPS)
	@echo $(MSG_EMPTYLINE)
	@echo $(MSG_COMPILING) 
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	$(REMOVE) $(OBJDIR)/*.o
	$(REMOVE) $(PROJECT)


APPNAME = "e[mulator] for PSP 0.82f"

CFG_PROFILE = 0

CFG_CORE_LNX = 1
CFG_CORE_SMS = 1
CFG_CORE_NGP = 1
CFG_CORE_GB  = 1
CFG_CORE_WS  = 1
CFG_CORE_PCE = 1
CFG_CORE_NES = 1

CORE_LIBRARY = 

DEF = $(CENV)

ifeq ($(CFG_PROFILE),1)
DEF += PROFILE
endif
LIBS_ADD = -Lobj 

#------------------------------------------------------------------------------
# LYNX SECTION
#------------------------------------------------------------------------------
ifeq ($(CFG_CORE_LNX),1)
  LNX_PATH= core/lynx
  LNX_LIB = obj/liblnx.a
  DEF += CORE_LNX
  LIBS_ADD += -llynx
  CORE_LIBRARY += $(LNX_LIB)
endif


#------------------------------------------------------------------------------
# SMS SECTION
#------------------------------------------------------------------------------
ifeq ($(CFG_CORE_SMS),1)
  SMS_PATH= core/sms
  SMS_LIB = obj/libsms.a
  DEF += CORE_SMS
  LIBS_ADD += -lsms
  CORE_LIBRARY += $(SMS_LIB)
endif

#------------------------------------------------------------------------------
# NES SECTION
#------------------------------------------------------------------------------
ifeq ($(CFG_CORE_NES),1)
  NES_PATH= core/nes
  NES_LIB = obj/libnes.a
  DEF += CORE_NES
  LIBS_ADD += -lnes
  CORE_LIBRARY += $(NES_LIB)
endif

#------------------------------------------------------------------------------
# GAMEBOY SECTION
#------------------------------------------------------------------------------
ifeq ($(CFG_CORE_GB),1)
  GBC_PATH= core/gb
  GBC_LIB = obj/libgbc.a
  DEF += CORE_GBC
  LIBS_ADD += -lgbc
  CORE_LIBRARY += $(GBC_LIB)
endif

#------------------------------------------------------------------------------
# NEOGEO POCKET SECTION
#------------------------------------------------------------------------------
ifeq ($(CFG_CORE_NGP),1)
  NGP_PATH= core/ngp
  NGP_LIB = obj/libngp.a
  DEF += CORE_NGP
  LIBS_ADD += -lngp
  CORE_LIBRARY += $(NGP_LIB)
endif

#------------------------------------------------------------------------------
# WONDERSWAN SECTION
#------------------------------------------------------------------------------
ifeq ($(CFG_CORE_WS),1)
  WSE_PATH = core/swan
  WSE_LIB = obj/libswan.a
  DEF += CORE_WS
  LIBS_ADD += -lswan
  CORE_LIBRARY += $(WSE_LIB)
endif

#------------------------------------------------------------------------------
# PC-Engine SECTION
#------------------------------------------------------------------------------
ifeq ($(CFG_CORE_PCE),1)
  PCE_PATH= core/pce
  PCE_LIB = obj/libpce.a
  DEF += CORE_PCE
  LIBS_ADD += -lpce
  CORE_LIBRARY += $(PCE_LIB)
endif


LIBS_DEF = libs/libmad.a libs/libunzip.z libs/libz.a
LIBS_DEFAULT = -Llibs -lmad -lunzip -lz

LIBS =  $(LIBS_DEFAULT) $(LIBS_ADD) 

# LIBS += -LF:/pspdev/psp/sdk/lib -lc -lpspkernel -lpspuser

INC = psp com libs/libzip libs/zlib libs/libmad
COM_PATH = com
PSP_PATH = psp

DEFS := $(addprefix -D,$(DEF))
INCS := $(addprefix -I,$(INC))


CFLAGS   = -include types.h $(COM_CFLAGS) $(INCS) $(DEFS) -c -o $@ $<
LDFLAGS  = -s -M -Ttext 8900000 -o out
#LDFLAGS  = -M -Ttext 8900000 -o out

PSPOBJ  = $(subst .c,.o,$(addprefix obj/,$(notdir $(wildcard $(PSP_PATH)/*.c))))
PSPLIBC = $(subst .c,.o,$(addprefix obj/,$(notdir $(wildcard $(PSP_PATH)/libc/*.c))))
PSPLIBC+= $(subst .s,.o,$(addprefix obj/,$(notdir $(wildcard $(PSP_PATH)/libc/*.s))))
COMOBJ  = $(subst .c,.o,$(addprefix obj/,$(notdir $(wildcard $(COM_PATH)/*.c))))

PSPHAL  = $(subst .c,.o,$(addprefix obj/,$(notdir $(wildcard $(PSP_PATH)/hal/*.c))))

OBJECTS = $(PSPLIBC) $(PSPOBJ) $(COMOBJ)  $(PSPHAL)

VPATH = psp:psp/libc:core:com:psp/hal:

all: $(OBJECTS) $(CORE_LIBRARY) $(LIBS_DEF) 
	$(LD) $(LDFLAGS) $(OBJECTS) $(LIBS) > obj/map.txt
	$(PAT) USERPROG
	$(E2P) outp $(APPNAME) "psp/ICON0.PNG"
	@rm out outp

obj/%.o: %.c
	$(CC) $(CFLAGS)

obj/%.o: %.s
	$(CC) $(CFLAGS)

clean:
	rm eboot.pbp obj/*.*

$(NES_LIB): FORCE
	make -C $(NES_PATH) lib

$(SMS_LIB): FORCE
	make -C $(SMS_PATH) lib

$(GBC_LIB): FORCE
	make -C $(GBC_PATH) lib

$(PCE_LIB): FORCE
	make -C $(PCE_PATH) lib

$(WSE_LIB): FORCE
	make -C $(WSE_PATH) lib

$(NGP_LIB): FORCE
	make -C $(NGP_PATH) lib

$(LNX_LIB): FORCE
	make -C $(LNX_PATH) lib

libs/libmad.a: FORCE
	make -C libs/libmad
	cp libs/libmad/libmad.a libs/

libs/libunzip.z: FORCE
	make -C libs/libzip
	cp libs/libzip/libunzip.a libs/

libs/libz.a: FORCE
	make -C libs/zlib
	cp libs/zlib/libz.a libs/


FORCE:


###############################################################################
##
include lib.mak

PSPSDK=$(shell psp-config --pspsdk-path)

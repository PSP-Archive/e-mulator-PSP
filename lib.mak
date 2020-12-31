###########################################################
##
## 
##
MAKE = make

CENV := ee

export CC  = $(CENV)-gcc
export CPP = $(CENV)-g++
export LD  = $(CENV)-ld
export AR  = $(CENV)-ar
PAT = outpatch
E2P = elf2pbp





###########################################################
# 
# core pathÇ©ÇÁÇÃëäëŒèÓïÒ
# 
#CORE_CFLAGS = -include $(EMULATOR)/com/types.h -I$(EMULATOR)/com -I./
CORE_CFLAGS = -include $(EMULATOR)/com/types.h -I$(EMULATOR)/com -I./
CORE_LIBPATH = $(EMULATOR)/obj


###########################################################
#
# common compile flag
#

export COM_CFLAGS = -O3 -mgp32 -fomit-frame-pointer -mlong32 -Wall

ifeq ($(CENV),ee)
  COM_CFLAGS += -march=r4000
  LDFLAG_COM= 
endif

###########################################################
##
## default 
##
#obj/%.o: %.c $(DEPH)
#	$(CC) $(CFLAGS) -c -o $@ $<
#
#$(LIB): $(OBJS)
#	$(AR) crus $(LIB) $(OBJS)




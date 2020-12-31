#BINARY = out

OBJ_NES = obj/nes6502.o \
          obj/nes_6502.o \
          obj/nes_ppu.o \
	      obj/nes_apu.o \
          obj/nes_apu_wrapper.o \
          obj/nes_exsound.o \
          obj/nes_fme7.o\
          obj/nes_mmc5.o \
          obj/nes_n106.o \
          obj/nes_vrc6.o \
          obj/nes_vrc7.o \
          obj/nes_fds.o \
	      obj/nes.o \
          obj/nes_config.o \
          obj/nes_mapper.o \
          obj/nes_rom.o \
          obj/snss.o \
          obj/nes_crc32.o \
          obj/libsnss.o \
          obj/debug.o

#          obj/fileio.o \
#          obj/string.o 

OBJ_PSP = obj/startup.o \
          obj/pg.o \
          obj/xmain.o

VPATH = debug:libsnss:cpu:ppu:apu:psp:com:libsnss:cpu

OBJS = $(OBJ_PSP) $(OBJ_NES) 

LIB_NES = ../../obj/libnes.a

CORE = ..

INC = cpu debug apu ppu libsnss mapper
INCS = $(addprefix -I,$(INC))

CFLAG = $(CORE_CFLAGS) $(COM_CFLAGS) $(INCS)
#CFLAG = $(INCS)


# all: $(LIB_NES) $(BINARY)

lib: $(LIB_NES)

$(LIB_NES): $(OBJ_NES) 
	$(AR) crus $(LIB_NES) $(OBJ_NES)

#$(BINARY): $(OBJS)
#	$(LD) -s -O3 $(OBJS) -M -Ttext 8900000 -q -o $@ > obj/NesterJ.map
#	$(PAT) USERPROG
#	$(E2P) "NesterJ NES Emulator 1.10"
#	rm out outp

obj/%.o : %.c
	$(CC) $(CFLAG) -c $< -o $@

obj/%.o : %.s
	$(CC) -march=r4000 -g -mgp32 -c -xassembler -O -o $@ $<

clean:
	del /s /f *.o *.map *.o.asm

##########################
##
##########################
include ../../lib.mak

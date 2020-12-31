LIB = $(CORE_LIBPATH)/libgbc.a

OBJS = obj/cpu.o \
       obj/gb.o  \
       obj/lcd.o \
       obj/sgb.o \
       obj/rom.o \
       obj/mbc.o \
       obj/apu.o \
       obj/renderer.o 

PSP_OBJS = obj/startup.o \
           obj/pg.o \
           obj/menu.o \
           obj/sound.o \
           obj/xmain.o \
           obj/string.o \
           obj/saveload.o 

CFLAGS = $(CORE_CFLAGS) $(COM_CFLAGS) -I./ $(DEFS)
VPATH = psp:

#all: $(LIB) out
lib: $(LIB)

#out: $(LIB) $(PSP_OBJS)
#	$(LD) -s -O0 $(PSP_OBJS) $(LIB_GBC) -M -Ttext 8900000 -q -o $@ > rin.map
#	$(PAT) USERPROG
#	$(E2P) outp "RIN GB/GBC Emulator"
#	rm out outp

#obj/%.o : %.c
#	$(CC) $(CFLAGS) -c $< -o $@

obj/%.o: %.c $(DEPH)
	$(CC) $(CFLAGS) -c -o $@ $<

$(LIB): $(OBJS)
	$(AR) crus $(LIB) $(OBJS)


obj/%.o : %.s
	$(CC) -march=r4000 -g -mgp32 -c -xassembler -O -o $@ $<

clean:
	del *.o /s


##########################
##
##########################
include ../../lib.mak


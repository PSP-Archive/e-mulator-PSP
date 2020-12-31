BINARY = out

# library object files common to compression and decompression
OBJECTS = bit.o decoder.o fixed.o frame.o huffman.o layer3.o layer12.o stream.o timer.o version.o synth.o

LIBRARY = libmad.a

all: $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	$(AR) crus $(LIBRARY) $(OBJECTS)

%.o : %.c
	$(CC) $(COM_CFLAGS) -DPSP -DNDEBUG -DFPM_DEFAULT -c $< -o $@

#%.o : %.s
#	$(CC) -march=r4000 -mgp32 -c -xassembler -O -o $@ $<

clean:
	del /f *.o *.map *.a

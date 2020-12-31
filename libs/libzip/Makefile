BINARY = out

OBJECTS = ioapi.o unzip.o zlibFileMemory.o zlibInterface.o 

all: $(BINARY)

$(BINARY): $(OBJECTS)
	$(AR) rcus libunzip.a $(OBJECTS)

INCS = -I..\zlib -I..\..\com

%.o : %.c
	$(CC) $(COM_CFLAGS) -DNOUNCRYPT $(INCS) -O3 -c $< -o $@

clean:
	del /s /f *.o *.map


#.SILENT: # ê√Ç©Ç…ÉRÉìÉpÉCÉã

SDIRS = .
SRC_S := $(foreach dir,$(SDIRS),$(wildcard $(dir)/*.s))
OBJ_S := $(notdir $(SRC_S:.s=.o))
SRC_C := $(foreach dir,$(SDIRS),$(wildcard $(dir)/*.c))
OBJ_C := $(notdir $(SRC_C:.c=.o))

OBJ = $(OBJ_S) $(OBJ_C)
LIBRARY = libc.a

DEFINE = 

CFLAG = -march=r4000 -O3 -mgp32 -fomit-frame-pointer -mlong32 -Wall
#CFLAG = -march=r4000 -O3 -g -mgp32 -fomit-frame-pointer -mlong32 -Wall
INCS  = -I..\pspsdk -I.

$(LIBRARY): $(OBJ)
	$(AR) crus $(LIBRARY) $(OBJ)
	mv $(LIBRARY) ../
	rm *.o

%.o: %.c
	$(CC) $(INCS) $(CFLAG) -c $< -o $@

%.o: %.s
	$(CC) $(INCS) $(CFLAG) -c $<  -o $@

clean:
	rm $(OBJ) $(LIBRARY)

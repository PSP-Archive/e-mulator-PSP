OBJS = $(subst .c,.o,$(addprefix obj/,$(notdir $(wildcard *.c))))
DEPH = $(notdir $(wildcard *.h))
LIB  = $(CORE_LIBPATH)/libngp.a

OBJS+= obj/interpret.o
OBJS+= obj/Z80n.o

VPATH = tlcs-900h:z80:

DEFS := $(addprefix -D,$(DEF))
INCS := $(addprefix -I,$(INC))

CFLAGS = $(CORE_CFLAGS) $(COM_CFLAGS) $(INCS)


all: $(LIB)
lib: $(LIB)

obj/%.o: %.c $(DEPH)
	$(CC) $(CFLAGS) -c -o $@ $<

$(LIB): $(OBJS)
	$(AR) crus $(LIB) $(OBJS)

clean:
	rm eboot.pbp obj/*.*


##########################
##
##########################
include ../../lib.mak


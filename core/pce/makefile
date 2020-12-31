OBJS = $(subst .c,.o,$(addprefix obj/,$(notdir $(wildcard *.c))))
DEPH = $(notdir $(wildcard *.h))
LIB  = $(CORE_LIBPATH)/libpce.a

CFLAGS = $(CORE_CFLAGS) $(COM_CFLAGS)


DEFS := $(addprefix -D,$(DEF))
INCS := $(addprefix -I,$(INC))

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


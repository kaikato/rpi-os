# all the tests
PROGS := $(wildcard final_render.c)
#PROGS := $(wildcard tests/[5]-test-receive.c)
OPT_LEVEL := -O3
LIBS += ./libgcc.a
CFLAGS_EXTRA := -DON_PI=1

COMMON_SRC := fb.c mb.c io.c vec3.c printf.c sqrt.c util.c scatter.c ray.c hit_record.c
# define this if you need to give the device for your pi
BOOTLOADER = my-install
RUN=1

# GREP_STR := 'HASH:\|ERROR:\|PANIC:\|SUCCESS:\|NRF:'
EXCLUDE ?= grep -v simple_boot


include $(CS140E_2023_PATH)/libpi/mk/Makefile.template-v2

kernel.elf: kernel.o $(ALL_OBJS) $(DEPS) $(START) $(LIBS)
	$(LD) $(LDFLAGS) $(START) $< $(ALL_OBJS) -T $(MEMMAP) -o $@  $(LIBS) $(LIB_POST)

kernel.list: kernel.elf
	$(OD) -d   kernel.elf > kernel.list

kernel.bin: kernel.list
	$(OCP) kernel.elf -O binary kernel.bin

final_render.elf: final_render.o $(ALL_OBJS) $(DEPS) $(START) $(LIBS)
	$(LD) $(LDFLAGS) $(START) $< $(ALL_OBJS) -T $(MEMMAP) -o $@  $(LIBS) $(LIB_POST)

final_render.list: final_render.elf
	$(OD) -d   final_render.elf > final_render.list

final_render.bin: final_render.list
	$(OCP) final_render.elf -O binary final_render.bin

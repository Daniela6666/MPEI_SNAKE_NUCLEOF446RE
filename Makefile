APPNAME = snake

CC = arm-none-eabi-gcc

CFLAGS = -Wall -Wextra -ffreestanding
CFLAGS += -mcpu=cortex-m4 -mthumb -mfloat-abi=soft
CFLAGS += -O0 -g
CFLAGS += -Iinc

LDFLAGS = -Tlinker_script.ld -nostdlib

C_SRC = main.c $(wildcard src/*.c)
ASM_SRC = crt0.S
OBJS = $(C_SRC:.c=.o) $(ASM_SRC:.S=.o)

all: $(APPNAME).elf

$(APPNAME).elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@
	arm-none-eabi-size $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o src/*.o *.elf

.PHONY: flash
flash: $(APPNAME).elf
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program $(APPNAME).elf verify reset exit"

SRCS = led.c blink.c

$(SUB_TARGET):$(SRCS:%c=%o)
	$(LD) $(LDFLAGS) $(SRCS:%c=%o) -r -o $@  
	echo "led"

%.o:%.c
	$(CC)  $(CFLAGS) -c  $<

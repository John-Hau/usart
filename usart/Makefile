SRCS = usart.c

$(SUB_TARGET):$(SRCS:%.c=%.o)
	$(LD) $(LDFLAGS)  $(SRCS:%.c=%.o) -r -o $@  
	echo "usart"

%.o:%.c
	$(CC)  $(CFLAGS) -c  $<

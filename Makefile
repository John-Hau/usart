ALL = app.exe

export SUB_TARGET = built-in.o

export CC = gcc
export CFLAGS = -Wall

export LD=ld
export LDPLAGS=

SRCS = main.c
SUB_DIR = usart led


#TOP_DIR=$(shell pwd)
#HEADER_DIR=$(TOP_DIR)/


$(ALL):$(SUB_DIR) $(SRCS:%.c=%.o)
	$(CC) $(CFLAGS) $(SRCS:%.c=%.o) $(SUB_DIR:=/$(SUB_TARGET)) -o $@ 

#%.d:%.c
#	$(CC) $(CFLAGS) -MM $< > $@
#sinclude $(SRC:.c=.d)

%.o:%.c
	$(CC) $(CFLAGS) -c $<

.PHONY: subdirs $(SUB_DIR)
subdirs:$(SUB_DIR)
	
$(SUB_DIR):
	make -C $@ 

.PHONY clean:
	rm -rf  $(ALL)




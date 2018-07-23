SOURCES:=$(wildcard *.c)
OBJS:=$(patsubst %.c,%.o,$(SOURCES))
ELF:=ftp_cli
CC:=gcc 
CFLAGS:=-Wall -I../include
$(ELF):$(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread -lcrypto
.PHONY:clean
clean:
	rm -rf $(ELF) $(OBJS)  


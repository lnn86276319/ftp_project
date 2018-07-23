SOURCES:=$(wildcard *.c)
OBJS:=$(patsubst %.c,%.o,$(SOURCES))
ELF:=ftp_ser
CC:=gcc 
CFLAGS:=-Wall -I../include
$(ELF):$(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lcrypt -lpthread -lmysqlclient
.PHONY:clean
clean:
	rm -rf $(ELF) $(OBJS)  


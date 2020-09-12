
CC := gcc
CFLAGS := -Iinclude -lkylestructs -lmysqlclient `mysql_config --cflags --libs`

SRC := ./src
INC := ./include

HDRS := $(INC)/*.h
DEPS := $(SRC)/*.c

default: all

all: $(DEPS)
	$(CC) $^ -o server.out $(CFLAGS)

debug: $(DEPS)
	$(CC) $^ -o serverdebug.out $(CFLAGS) -g

clean:
	if [ -f "server" ]; then rm server; fi;


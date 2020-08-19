
CC := gcc

CFLAGS := -I ./include -g -lkylestructs -lmysqlclient `mysql_config --cflags --libs`

default: all


SERVER := falcon.out

DEPS := src/server.c src/client_manager.c src/http_delete.c src/http_get.c src/http_post.c src/http_put.c 

all: $(DEPS) 
	$(CC) $^ -o $(SERVER) $(CFLAGS)


clean:
	if [ -f "falcon.out" ]; then rm falcon.out; fi;




CC := gcc

CFLAGS := -g -lkylestructs

default: all


SERVER := falcon.out

DEPS := server.c conn_mgr.c http_delete.c http_get.c http_post.c http_put.c 

all: $(DEPS) 
	$(CC) $^ -o $(SERVER) $(CFLAGS)


clean:
	if [ -f "falcon.out" ]; rm falcon.out; fi;



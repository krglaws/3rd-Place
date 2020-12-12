
CC := gcc
CFLAGS := -Iinclude -lkylestructs -lmysqlclient `mysql_config --cflags --libs`
DEBUG := -g

SRC := ./src
INC := ./include
BIN := ./bin

default: server.out

CLIMGRDEP := $(SRC)/client_manager.c
CLIMGRTARG := $(BIN)/client_manager.o

$(CLIMGRTARG): $(CLIMGRDEP)
	$(CC) -c $< -o $@ $(CFLAGS) $(DEBUG)


HTPDELDEP := $(SRC)/http_delete.c
HTPDELTARG := $(BIN)/http_delete.o

$(HTPDELTARG): $(HTPDELDEP)
	$(CC) -c $< -o $@ $(CFLAGS) $(DEBUG)


HTPGETDEP := $(SRC)/http_get.c
HTPGETTARG := $(BIN)/http_get.o

$(HTPGETTARG): $(HTPGETDEP)
	$(CC) -c $< -o $@ $(CFLAGS) $(DEBUG)


HTPPSTDEP := $(SRC)/http_post.c
HTPPSTTARG := $(BIN)/http_post.o

$(HTPPSTTARG): $(HTPPSTDEP)
	$(CC) -c $< -o $@ $(CFLAGS) $(DEBUG)


HTPPUTDEP := $(SRC)/http_put.c
HTPPUTTARG := $(BIN)/http_put.o

$(HTPPUTTARG): $(HTPPUTDEP)
	$(CC) -c $< -o $@ $(CFLAGS) $(DEBUG)


SNDERRDEP := $(SRC)/senderr.c
SNDERRTARG := $(BIN)/senderr.o

$(SNDERRTARG): $(SNDERRDEP)
	$(CC) -c $< -o $@ $(CFLAGS) $(DEBUG)


SVRDEP := $(SRC)/server.c
SVRTARG := $(BIN)/server.o

$(SVRTARG): $(SVRDEP)
	$(CC) -c $< -o $@ $(CFLAGS) $(DEBUG)


SQLMGRDEP := $(SRC)/sql_manager.c
SQLMGRTARG := $(BIN)/sql_manager.o

$(SQLMGRTARG): $(SQLMGRDEP)
	$(CC) -c $< -o $@ $(CFLAGS) $(DEBUG)


SQLWPRDEP := $(SRC)/sql_wrapper.c
SQLWPRTARG := $(BIN)/sql_wrapper.o

$(SQLWPRTARG): $(SQLWPRDEP)
	$(CC) -c $< -o $@ $(CFLAGS) $(DEBUG)


TKNMGRDEP := $(SRC)/token_manager.c
TKNMGRTARG := $(BIN)/token_manager.o

$(TKNMGRTARG): $(TKNMGRDEP)
	$(CC) -c $< -o $@ $(CFLAGS) $(DEBUG)


UTILDEP := $(SRC)/util.c
UTILTARG := $(BIN)/util.o

$(UTILTARG): $(UTILDEP)
	$(CC) -c $< -o $@ $(CFLAGS) $(DEBUG)


ALLDEPS := $(UTILTARG) $(SQLMGRTARG) $(SQLWPRTARG) $(TKNMGRTARG) $(CLIMGRTARG) \
	$(SNDERRTARG) $(HTPDELTARG) $(HTPGETTARG) $(HTPPSTTARG) $(HTPPUTTARG) $(SVRTARG)

.PHONY: server.out
server.out: $(ALLDEPS)
	$(CC) $^ -o $@ $(CFLAGS) $(DEBUG)


.PHONY: clean
clean:
	if [ -f "server.out" ]; then rm server.out; fi;
	rm -r ./bin/*.o


.PHONY: all
all: server.out

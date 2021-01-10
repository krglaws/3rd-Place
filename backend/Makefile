
CC := gcc
CFLAGS := -Iinclude -lkylestructs -lmysqlclient `mysql_config --cflags --libs` -lcrypt
DEBUG := -g -DDEBUG

SRC := ./src
INC := ./include
BIN := ./bin

default: all

SOCKMGRDEP := $(SRC)/socket_manager.c
SOCKMGRTARG := $(BIN)/socket_manager.o

$(SOCKMGRTARG): $(SOCKMGRDEP)
	$(CC) -c $< -o $@ $(CFLAGS) $(DEBUG)


STRINGMAPDEP := $(SRC)/string_map.c
STRINGMAPTARG := $(BIN)/string_map.o

$(STRINGMAPTARG): $(STRINGMAPDEP)
	$(CC) -c $< -o $@ $(CFLAGS) $(DEBUG)


HTPDELDEP := $(SRC)/http_delete.c
HTPDELTARG := $(BIN)/http_delete.o

$(HTPDELTARG): $(HTPDELDEP)
	$(CC) -c $< -o $@ $(CFLAGS) $(DEBUG)


HTPGETDEP := $(SRC)/http_get.c
HTPGETTARG := $(BIN)/http_get.o

$(HTPGETTARG): $(HTPGETDEP)
	$(CC) -c $< -o $@ $(CFLAGS) $(DEBUG)


GETUSRDEP := $(SRC)/get_user.c
GETUSRTARG := $(BIN)/get_user.o

$(GETUSRTARG): $(GETUSRDEP)
	$(CC) -c $< -o $@ $(CFLAGS) $(DEBUG)


GETPSTDEP := $(SRC)/get_post.c
GETPSTTARG := $(BIN)/get_post.o

$(GETPSTTARG): $(GETPSTDEP)
	$(CC) -c $< -o $@ $(CFLAGS) $(DEBUG)


GETCMTYDEP := $(SRC)/get_community.c
GETCMTYTARG := $(BIN)/get_community.o

$(GETCMTYTARG): $(GETCMTYDEP)
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


AUTHMGRDEP := $(SRC)/auth_manager.c
AUTHMGRTARG := $(BIN)/auth_manager.o

$(AUTHMGRTARG): $(AUTHMGRDEP)
	$(CC) -c $< -o $@ $(CFLAGS) $(DEBUG)


UTILDEP := $(SRC)/util.c
UTILTARG := $(BIN)/util.o

$(UTILTARG): $(UTILDEP)
	$(CC) -c $< -o $@ $(CFLAGS) $(DEBUG)


LOGMGRDEP := $(SRC)/log_manager.c
LOGMGRTARG := $(BIN)/log_manager.o

$(LOGMGRTARG): $(LOGMGRDEP)
	$(CC) -c $< -o $@ $(CFLAGS) $(DEBUG)


ALLDEPS := $(UTILTARG) $(STRINGMAPTARG) $(LOGMGRTARG) $(SQLMGRTARG) $(AUTHMGRTARG) \
	$(SOCKMGRTARG) $(SNDERRTARG) $(HTPDELTARG) $(HTPGETTARG) $(GETUSRTARG) $(GETPSTTARG) $(GETCMTYTARG) \
	$(HTPPSTTARG) $(HTPPUTTARG) $(SVRTARG)


.PHONY: server
server: $(ALLDEPS)
	$(CC) $^ -o $@ $(CFLAGS) $(DEBUG)


.PHONY: clean
clean:
	if [ -f "server" ]; then rm server; fi;
	rm -r ./bin/*.o


.PHONY: all
all: server

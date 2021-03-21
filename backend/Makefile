CC := gcc
CFLAGS := -Iinclude -g -Wall -Wextra

SRC := ./src
INC := ./include
BIN := ./bin

default: all

VALIDATORDEP := $(SRC)/validator.c
VALIDATORTARG := $(BIN)/validator.o

$(VALIDATORTARG): $(VALIDATORDEP)
	$(CC) -c $< -o $@ $(CFLAGS) 


SOCKMGRDEP := $(SRC)/socket_manager.c
SOCKMGRTARG := $(BIN)/socket_manager.o

$(SOCKMGRTARG): $(SOCKMGRDEP)
	$(CC) -c $< -o $@ $(CFLAGS) 


STRINGMAPDEP := $(SRC)/string_map.c
STRINGMAPTARG := $(BIN)/string_map.o

$(STRINGMAPTARG): $(STRINGMAPDEP)
	$(CC) -c $< -o $@ $(CFLAGS) 


TEMPLATINGDEP := $(SRC)/templating.c
TEMPLATINGTARG := $(BIN)/templating.o

$(TEMPLATINGTARG): $(TEMPLATINGDEP)
	$(CC) -c $< -o $@ $(CFLAGS) 


HTPDELDEP := $(SRC)/http_delete.c
HTPDELTARG := $(BIN)/http_delete.o

$(HTPDELTARG): $(HTPDELDEP)
	$(CC) -c $< -o $@ $(CFLAGS) 


HTPGETDEP := $(SRC)/http_get.c
HTPGETTARG := $(BIN)/http_get.o

$(HTPGETTARG): $(HTPGETDEP)
	$(CC) -c $< -o $@ $(CFLAGS) 


GETFORMDEP := $(SRC)/get_form.c
GETFORMTARG:= $(BIN)/get_form.o

$(GETFORMTARG): $(GETFORMDEP)
	$(CC) -c $< -o $@ $(CFLAGS) 


GETUSRDEP := $(SRC)/get_user.c
GETUSRTARG := $(BIN)/get_user.o

$(GETUSRTARG): $(GETUSRDEP)
	$(CC) -c $< -o $@ $(CFLAGS) 


GETPSTDEP := $(SRC)/get_post.c
GETPSTTARG := $(BIN)/get_post.o

$(GETPSTTARG): $(GETPSTDEP)
	$(CC) -c $< -o $@ $(CFLAGS) 


GETFEEDDEP := $(SRC)/get_feed.c
GETFEEDTARG := $(BIN)/get_feed.o

$(GETFEEDTARG): $(GETFEEDDEP)
	$(CC) -c $< -o $@ $(CFLAGS) 


GETCMTYDEP := $(SRC)/get_community.c
GETCMTYTARG := $(BIN)/get_community.o

$(GETCMTYTARG): $(GETCMTYDEP)
	$(CC) -c $< -o $@ $(CFLAGS) 


HTPPSTDEP := $(SRC)/http_post.c
HTPPSTTARG := $(BIN)/http_post.o

$(HTPPSTTARG): $(HTPPSTDEP)
	$(CC) -c $< -o $@ $(CFLAGS) 


HTPPUTDEP := $(SRC)/http_put.c
HTPPUTTARG := $(BIN)/http_put.o

$(HTPPUTTARG): $(HTPPUTDEP)
	$(CC) -c $< -o $@ $(CFLAGS) 


RESPONSEDEP := $(SRC)/response.c
RESPONSETARG := $(BIN)/response.o

$(RESPONSETARG): $(RESPONSEDEP)
	$(CC) -c $< -o $@ $(CFLAGS) 


SVRDEP := $(SRC)/server.c
SVRTARG := $(BIN)/server.o

$(SVRTARG): $(SVRDEP)
	$(CC) -c $< -o $@ $(CFLAGS) 


SQLMGRDEP := $(SRC)/sql_manager.c
SQLMGRTARG := $(BIN)/sql_manager.o

$(SQLMGRTARG): $(SQLMGRDEP)
	$(CC) -c $< -o $@ $(CFLAGS) `mysql_config --cflags`


AUTHMGRDEP := $(SRC)/auth_manager.c
AUTHMGRTARG := $(BIN)/auth_manager.o

$(AUTHMGRTARG): $(AUTHMGRDEP)
	$(CC) -c $< -o $@ $(CFLAGS) 


LOADFILEDEP := $(SRC)/load_file.c
LOADFILETARG := $(BIN)/load_file.o

$(LOADFILETARG): $(LOADFILEDEP)
	$(CC) -c $< -o $@ $(CFLAGS) 


LOGMGRDEP := $(SRC)/log_manager.c
LOGMGRTARG := $(BIN)/log_manager.o

$(LOGMGRTARG): $(LOGMGRDEP)
	$(CC) -c $< -o $@ $(CFLAGS) 


ALLDEPS := $(LOADFILETARG) $(STRINGMAPTARG) $(TEMPLATINGTARG) $(LOGMGRTARG) $(SQLMGRTARG) $(AUTHMGRTARG) \
	$(SOCKMGRTARG) $(RESPONSETARG) $(HTPDELTARG) $(HTPGETTARG) $(GETUSRTARG) $(GETPSTTARG) $(GETCMTYTARG) \
	$(HTPPSTTARG) $(HTPPUTTARG) $(SVRTARG) $(GETFEEDTARG) $(GETFORMTARG) $(VALIDATORTARG)


server: $(ALLDEPS)
	$(CC) -o $@ $^ -lkylestructs -lcrypt `mysql_config --libs`


.PHONY: clean
clean:
	rm bin/*.o server


.PHONY: all
all: server

CC = g++
RM = rm -rf
MKDIR = mkdir -p
DIR = ./bin
INCDIR = ./inc
INC = $(wildcard $(INCDIR)/*.h)
INC += $(wildcard $(INCDIR)/*.hpp)
SRC = $(wildcard ./example/*.cpp)
LIB = -lpthread -lboost_system -lboost_filesystem -lboost_thread \
	  -lboost_program_options
BIN = ./bin/server

all: $(DIR) $(BIN)

$(BIN): $(SRC) $(INC)
	$(CC) $(SRC) -I$(INCDIR) -L./lib $(LIB) -o $@

$(DIR):
	$(MKDIR) $@

PHONY: clean
clean:
	$(RM) $(BIN)
	$(RM) $(DIR)

TARGET := hello
LIB_NAME := libcdata.so
LIB_SRC_DIRS := src
EXE_SRC_DIRS := ./ testcase
INCLUDE_DIRS := -I./include -I./ -I./src -I./testcase
CXXFLAGS :=
CCFLAGS :=

CXX := g++
CC := gcc
ECHO := echo
RM := rm -rf
MKDIR := mkdir -p

OBJ_DIR := obj
BIN_DIR := bin
LIB_DIR := lib
COMILE_GOAL :=

LIB_SRC_FILES := $(foreach dir, $(LIB_SRC_DIRS), $(notdir $(wildcard $(dir)/*.c)))
LIB_OBJ_FILES := $(patsubst %.c, %.o, $(LIB_SRC_FILES))

EXE_SRC_FILES := $(foreach dir, $(EXE_SRC_DIRS), $(notdir $(wildcard $(dir)/*.c)))
EXE_OBJ_FILES := $(patsubst %.c, %.o, $(EXE_SRC_FILES))

ifeq ($(MAKECMDGOALS), release)
    CXXFLAGS += -D_RELEASE_VERSION_  -D_DEBUG_LEVEL_=3 -O2
    CCFLAGS += -D_RELEASE_VERSION_ -D_DEBUG_LEVEL_=3 -O2
	COMILE_GOAL := release
else
    CXXFLAGS += -D_DEBUG_ALL_PRINT_BUFFER_ON_ -D_DEBUG_LEVEL_=0 -Wall -Wformat -g
    CCFLAGS +=  -D_DEBUG_ALL_PRINT_BUFFER_ON_ -D_DEBUG_LEVEL_=0 -Wall -Wformat -g
	COMILE_GOAL := debug
endif

OBJ_DIR := $(OBJ_DIR)/$(COMILE_GOAL)
LIB_OBJ_FULL_PATH_FILES := $(addprefix $(OBJ_DIR)/, $(LIB_OBJ_FILES))
EXE_OBJ_FULL_PATH_FILES := $(addprefix $(OBJ_DIR)/, $(EXE_OBJ_FILES))

BIN_DIR := $(BIN_DIR)/$(COMILE_GOAL)
LIB_DIR := $(LIB_DIR)/$(COMILE_GOAL)

FULL_PATH_LIB_FILE  := $(addprefix $(LIB_DIR)/, $(LIB_NAME))
	
vpath %.c $(LIB_SRC_DIRS)
vpath %.c $(EXE_SRC_DIRS)
vpath %.o $(OBJ_DIR)

CXXFLAGS +=  $(INCLUDE_DIRS) -std=c++0x -Wl,-rpath=$(LIB_DIR)
CCFLAGS +=  $(INCLUDE_DIRS) -Wl,-rpath=$(LIB_DIR)

.PHONY:all COMPILE_LIB COMPILE_EXECUTE MK_OBJ_DIR

all release:MK_ALL_DIRS COMPILE_LIB COMPILE_EXECUTE

clean:
	@$(ECHO) "Clean object files..."
	@$(RM) $(OBJ_DIR)/*
	@$(ECHO) "Clean so file..."
	@$(RM) $(LIB_DIR)/*
	@$(ECHO) "Clean execute file..."
	@$(RM) $(BIN_DIR)/*
	@$(ECHO) "Clean done!"


MK_ALL_DIRS:
	@$(MKDIR) $(OBJ_DIR)
	@$(MKDIR) $(LIB_DIR)
	@$(MKDIR) $(BIN_DIR)
	
COMPILE_LIB:$(LIB_OBJ_FILES)
	@$(ECHO) "Compiling libcdata.so ..."
	$(CC)  -shared $(LIB_OBJ_FULL_PATH_FILES) -o $(FULL_PATH_LIB_FILE) -lrt
	@$(ECHO) "Done!"
	@$(ECHO) 
	
DEBUG_SHOW_VALUE:
	@$(ECHO) "========================SRC File============================="
	@$(ECHO) $(LIB_SRC_FILES)
	@$(ECHO) "========================================================"
	@$(ECHO)
	@$(ECHO) "========================OBJ File============================="
	@$(ECHO) $(LIB_OBJ_FILES)
	@$(ECHO) "========================================================"
	@$(ECHO)
	
    
COMPILE_EXECUTE:$(EXE_OBJ_FILES)
	@$(ECHO) "Compiling execute file..." 
	$(CC) $(CCFLAGS) $(EXE_OBJ_FULL_PATH_FILES) -o $(BIN_DIR)/$(TARGET) -L$(LIB_DIR) -lpthread -lrt -lcdata
	@$(ECHO) "Done!"
	@$(ECHO)
	
%.o:%.c
	@$(ECHO) Compiling:$<...  
	@$(CC) $(CCFLAGS) -fPIC -o $(OBJ_DIR)/$@ -c $<
	@$(ECHO) done!
	@$(ECHO)



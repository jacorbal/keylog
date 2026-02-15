# Makefile
#
# Author: J. A. Corbal (<jacorbal@gmail.com>)

## Directories
PWD   = $(CURDIR)
I_DIR = ${PWD}/include
S_DIR = ${PWD}/src
L_DIR = ${PWD}/lib
O_DIR = ${PWD}/obj
B_DIR = ${PWD}/bin

SHELL=/bin/sh

## Compiler & linker options
CCSTD       = c11 # c89 | c90, c99, c11, c17, gnu11, gnu17,...
CCOPT       = 3   # 0:debug; 1:optimize; 2:optimize more; 3:yet more
CCOPTS      = -pedantic -pedantic-errors
CCEXTRA     = -fdiagnostics-color=always -fdiagnostics-show-location=once
CCWARN_TINY = -Wpedantic -Wall -Wextra -Wshadow -Wundef -Werror
CCWARN_MORE = -Wwrite-strings -Wconversion -Wdouble-promotion
CCWARN_MOST = -Wformat -Wuninitialized -Wfloat-equal \
				-Wcast-align -Wpointer-arith -Wstrict-overflow=5 \
				-Wunreachable-code -Wmissing-format-attribute \
				-Wdeprecated \
				-Wno-padded -Wno-unused-parameter -Wno-format-nonliteral
CCWARN_GCC  = -Wlogical-op -Wstrict-aliasing=3 -Wduplicated-branches \
				-Wformat-overflow -Wformat-signedness -Wstrict-aliasing=3 \
				-Wno-suggest-attribute=format
CCWARN_CLANG = -Wbad-function-cast -Wextra-semi-stmt -Wmissing-prototypes \
				-Wswitch-enum -Wcovered-switch-default -Wreserved-identifier \
				-Wdeclaration-after-statement -Wsometimes-uninitialized \
				-Wdocumentation \
				-Wno-fortify-source -Wno-cast-align -Wno-cast-qual
CCWARN      = ${CCWARN_TINY} ${CCWARN_MORE} ${CCWARN_MOST}
CCFLAGS     = ${CCOPTS} ${CCWARN} -std=${CCSTD} ${CCEXTRA} -I ${I_DIR}
LDFLAGS     = -L ${L_DIR}
LDJFLAGS    = -L ${L_DIR} -lcjson

## Options on `make`
# Compiler: `make clean && make CC=clang` or `make clean && make CC=gcc`
CC = gcc
ifeq ($(CC), clang)
	CCWARN += ${CCWARN_CLANG}
else ifeq ($(CC), gcc)
	CCWARN += ${CCWARN_GCC}
else
	$(error Unsupported compiler '$(CC)': CC only admits 'gcc' or 'clang')
endif

# Use `make clean && make DEBUG=1` to add debugging information
# Use `make clean && make DEBUG=2` to also link with the address sanitizer 
DEBUG ?= 0
ifeq ($(DEBUG), 1)
	CCFLAGS += -DDEBUG -g3 -ggdb3 -O0
else ifeq ($(DEBUG), 2)
	CCFLAGS += -DDEBUG -g3 -ggdb3 -O0
	LDFLAGS += -fsanitize=address -fno-omit-frame-pointer -fPIC
else
	CCFLAGS += -DNDEBUG -O${CCOPT}
endif

# Use `make clean && make STRIP=1` to discard symbols from object files
STRIP ?= 0
ifeq ($(STRIP), 1)
	LDFLAGS += -s
endif


## Makefile files & directories
SHELL = /bin/sh
.SUFFIXES:
.SUFFIXES: .h .c .o

# Binary file options and running arguments
KLTARGET = ${B_DIR}/keylog
KLSTARGET = ${B_DIR}/klserver
KLJTARGET = ${B_DIR}/kljson
ARGS ?=

# Sources and objects
KLSRCS = ${S_DIR}/keycodes.c ${S_DIR}/kbdfile.c ${S_DIR}/keylog.c \
    ${S_DIR}/network.c ${S_DIR}/main.c
KLSSRCS = ${S_DIR}/tools/klserver.c ${S_DIR}/network.c
KLJSRCS = ${S_DIR}/tools/kljson.c
KLOBJS = $(patsubst ${S_DIR}/%.c, ${O_DIR}/%.o, $(KLSRCS))
KLSOBJS = $(patsubst ${S_DIR}/%.c, ${O_DIR}/%.o, $(KLSSRCS))
KLJOBJS = $(patsubst ${S_DIR}/%.c, ${O_DIR}/%.o, $(KLJSRCS))

# Linkage of keylogger
${KLTARGET}: ${KLOBJS}
	${CC} -o $@ $^ ${LDFLAGS}

# Linkage of keylogger server
${KLSTARGET}: ${KLSOBJS}
	${CC} -o $@ $^ ${LDFLAGS}

# Linkage of keylogger JSON converter
${KLJTARGET}: ${KLJOBJS}
	${CC} -o $@ $^ ${LDJFLAGS}

# Compilation
${O_DIR}/%.o: ${S_DIR}/%.c
	${CC} ${CCFLAGS} -c -o $@ $<


## Make options
.PHONY: keylog klserver kljson ctags clean clean-obj clean-bin run \
        hard hard-run help

keylog:
	@make mkdirs
	make ${KLTARGET}
	@make ctags

klserver:
	@make mkdirs
	make ${KLSTARGET}
	@make ctags

kljson:
	@make mkdirs
	make ${KLJTARGET}
	@make ctags

mkdirs:
	@if [ ! -d $(B_DIR) ]; then mkdir -p $(B_DIR); fi
	@if [ ! -d $(O_DIR) ]; then mkdir -p $(O_DIR); fi
	@if [ ! -d $(O_DIR)/tools ]; then mkdir -p $(O_DIR)/tools; fi 

all:
	make keylog
	make klserver
	make kljson

ctags:
ifeq (,$(wildcard "/usr/bin/ctags"))
	@echo "Generating tags..."
	@ctags -R --exclude='doc' --exclude='obj' --exclude='_tmp_' .
	@echo "Tag files generated"
else
	$(error Cannot find '/usr/bin/ctags')
endif

clean-obj:
	rm -f ${KLOBJS}
	rm -f ${KLSOBJS}
	rm -f ${KLJOBJS}

clean-bin:
	rm -f ${KLTARGET}
	rm -f ${KLSTARGET}
	rm -f ${KLJTARGET}

clean:
	@make clean-obj
	@make clean-bin

run:
	${KLTARGET} ${ARGS}

hard:
	@make clean
	@make all

hard-run:
	@make hard
	@make run

help:
	@echo "Command:"
	@echo "  make keylog            Build keylogger (default)"
	@echo "  make klserver          Build keylogger server"
	@echo "  make kljson            Build keylogger JSON converter"
	@echo "  make all               Build project (keylog & klserver & kljson)"
	@echo "  make clean-obj         Clean object files"
	@echo "  make clean             Clean binaries and object files"
	@echo "  make ctags             Generate tag files for source"
	@echo "  make hard              Clean and build"
	@echo "  make run               Run keylogger binary (if exists)"
	@echo "  make run ARGS=<args>   Run with arguments (if binary exists)"
	@echo "  make hard-run          Clean, build and run (if binary exists)"
	@echo
	@echo "Options:"
	@echo "  Use 'DEBUG=1' to generate detailed debug information"
	@echo "  Use 'DEBUG=2' to also link with address sanitizer"
	@echo "  Use 'CC=<compiler>' to select a compiler ('gcc' or 'clang')"
	@echo
	@echo "Binaries will be placed in:"
	@echo "  '${KLTARGET}'"
	@echo "  '${KLSTARGET}'"
	@echo "  '${KLJTARGET}'"

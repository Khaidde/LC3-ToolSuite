PLATFORM := $(shell uname -s)
ifneq ($(findstring MSYS,$(PLATFORM)),)
PLATFORM := windows32
endif

ifneq ($(PLATFORM),windows32)
$(warn Build not test on ${PLATFORM})
MAKE := make
MKDIR := mkdir
RM := rm
else
MAKE := $(shell which make)
MKDIR := $(shell which mkdir)
RM := $(shell which rm)
endif

ifeq ($(shell which clang++),)
$(error Could not find path to clang++)
else
CC := clang++
CFLAGS += -g
CFLAGS += -std=c++17 -Wall -Wextra -Werror -Wsign-conversion
CFLAGS += -I./utils/src
endif

BIN_DIR ?= bin
OBJ_DIR ?= build/obj

ifdef DEPS
-include ${DEPS}
endif

get-objs = $(patsubst $1/src/%.cpp,$1/$(OBJ_DIR)/%.o, $(shell ls $1/src/*.cpp))
get-deps = $(patsubst $1/src/%.cpp,$1/$(OBJ_DIR)/%.d, $(shell ls $1/src/*.cpp))
clean-dir = ${RM} -f $1/${OBJ_DIR}/*

DIRECTORIES := lcasm lcc lcemu

.PHONY: build-all ${DIRECTORIES} clean

build-all: utils ${DIRECTORIES}

${DIRECTORIES}: utils
	@printf "\nBuilding $@...\n"
	@${MAKE} ${BIN_DIR}/$@.exe DIR=$@ OBJS="$(call get-objs,$@)" DEPS="$(call get-deps,$@)"

utils: force
	@printf "\nBuilding $@...\n"
	$(eval OBJS := $(call get-objs,$@))
	@${MAKE} ${OBJS} DIR=$@ OBJS="${OBJS}" DEPS="$(call get-deps,$@)"

force:

${BIN_DIR}/${DIR}.exe: ${OBJS} $(call get-objs,utils)
	@${MKDIR} -p ${dir $@}
	${CC} ${CFLAGS} $^ -o $@

${DIR}/build/obj/%.o: ${DIR}/src/%.cpp
	@${MKDIR} -p ${dir $@}
	${CC} ${CFLAGS} -c $< -MMD -MF $(@:.o=.d) -o $@

example:
	lcc ./lcc/examples/main.c ./lcc/examples/main.asm
	lcasm ./lcc/examples/main.asm ./lcc/examples/main.bin
	lcemu ./lcc/examples/main.bin

clean:
	${RM} -f ${BIN_DIR}/*
	$(call clean-dir,lcasm)
	$(call clean-dir,lcemu)
	$(call clean-dir,utils)

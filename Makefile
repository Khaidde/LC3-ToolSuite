
PLATFORM := $(shell uname -s)
ifneq ($(findstring MSYS,$(PLATFORM)),)
PLATFORM := windows32
endif

ifneq ($(PLATFORM),windows32)
${warn Build not test on ${PLATFORM}}
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
CFLAGS += -O3
CFLAGS += -std=c++17 -Wall -Wextra -Werror -Wsign-conversion
endif

OBJ_DIR := build/obj

.PHONY: build-all lib asm clean-default

lib:
	+${MAKE} -C lib build

asm:
	+${MAKE} -C asm build

build-all: lib asm

clean-default:
	+${MAKE} -C lib clean
	+${MAKE} -C asm clean

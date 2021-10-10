-include ../Makefile

BIN_DIR ?= build/bin
OBJ_DIR ?= build/obj
EXEC := lcasm.exe

SOURCES := $(shell ls src/*.cpp)
OBJECTS := $(patsubst src/%.cpp, $(OBJ_DIR)/%.o, $(SOURCES))
DEPS := $(patsubst src/%.cpp, $(OBJ_DIR)/%.d, $(SOURCES))
-include ${DEPS}

.PHONY: build clean

build: ${BIN_DIR}/${EXEC}

${BIN_DIR}/${EXEC}: ${OBJECTS}
	@${MKDIR} -p ${dir $@}
	${CC} ${CFLAGS} $^ ../lib/build/obj/general.o -o $@

${OBJ_DIR}/%.o: src/%.cpp
	@${MKDIR} -p ${dir $@}
	${CC} ${CFLAGS} -I../lib/src -c $< -MD -MF $(@:.o=.d) -o $@

clean:
	${RM} -f ${OBJ_DIR}/* ${BIN_DIR}/*

-include ../Makefile

.PHONY: build clean

build: ${OBJ_DIR}/general.o

${OBJ_DIR}/general.o: src/general.cpp src/general.hpp
	@${MKDIR} -p ${dir $@}
	${CC} ${CFLAGS} -c $< -o $@

clean:
	${RM} -f ${OBJ_DIR}/*

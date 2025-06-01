
# PYTHON_PREFIX := $(shell brew --prefix python)
# C_INCLUDE="${PYTHON_PREFIX}/Frameworks/Python.framework/Versions/Current/include"
# C_LIB="${PYTHON_PREFIX}/Frameworks/Python.framework/Versions/Current/lib"

PYTHON_PREFIX := $(shell brew --prefix python)
PYTHON_CONFIG_BIN = "${PYTHON_PREFIX}/bin/python3-config"
INCLUDE_FLAGS := $(shell ${PYTHON_CONFIG_BIN} --include)
LD_FLAGS := $(shell ${PYTHON_CONFIG_BIN} --ldflags --embed)

# C_INCLUDE := $(shell pkg-config --cflags --static python3)
# LD_FLAGS := $(shell pkg-config --libs --static python3)


.PHONY: all build exec clean

all: clean exec

include:
	@mkdir include

build:
	@mkdir build

include/config.h: include
	@printf "#ifndef _PY_INTERPRETER_CONFIG_H_\n#define _PY_INTERPRETER_CONFIG_H_\n\n" > $@
	@xxd -i --name python_source ./src/hello.py >> $@
	@printf "\n#endif\n" >> $@

build/python_interpreter.o: build include/config.h
	@gcc ${INCLUDE_FLAGS} -c -Iinclude src/python_interpreter.c -o $@

build/main: build build/python_interpreter.o
# @gcc -I${C_INCLUDE} -L${C_LIB} -lpython3.13 -fno-strict-overflow -Wsign-compare -Wunreachable-code -fno-common -dynamic -DNDEBUG -g -O3 -Wall ./c_project/main.c -o ./c_project/main
	@gcc ${INCLUDE_FLAGS} -Iinclude ${LD_FLAGS} build/python_interpreter.o src/main.c -o $@

exec: build/main
	@./build/main

clean:
	@rm -rf ./build ./include/config.h || true



# Check if the system has homebrew installed.
HAS_HOMEBREW := $(shell command -v brew 2> /dev/null)

# Find python lib and headers.
ifdef HAS_HOMEBREW
# use homebrew to find python
PYTHON_PREFIX := $(shell brew --prefix python)
PYTHON_CONFIG_BIN = "${PYTHON_PREFIX}/bin/python3-config"
INCLUDE_FLAGS := $(shell ${PYTHON_CONFIG_BIN} --include)
LD_FLAGS := $(shell ${PYTHON_CONFIG_BIN} --ldflags --embed)
else
# Use pkg-config to find python
INCLUDE_FLAGS := $(shell pkg-config --cflags --static python3)
LD_FLAGS := $(shell pkg-config --libs --static python3)
LD_FLAGS += -lpython3.13
endif

# C Compiler flags, extracted from `python3-config --cflags --embed`
CCFLAGS := -fno-strict-overflow -Wsign-compare -Wunreachable-code -fno-common -dynamic -DNDEBUG -g -O3 -Wall

.PHONY: all exec clean

all: clean exec

include/config.h:
	@printf "#ifndef _PY_INTERPRETER_CONFIG_H_\n#define _PY_INTERPRETER_CONFIG_H_\n\n" > $@
	@xxd -i --name python_source ./src/hello.py >> $@
	@printf "\n#endif\n" >> $@

build/python_interpreter.o: build include/config.h
	@gcc ${INCLUDE_FLAGS} ${CCFLAGS} -c -Iinclude src/python_interpreter.c -o $@

build/main: build build/python_interpreter.o
	@gcc ${INCLUDE_FLAGS} -Iinclude ${LD_FLAGS} ${CCFLAGS} build/python_interpreter.o src/main.c -o $@

build:
	@mkdir build

exec: build/main
	@./build/main

clean:
	@rm -rf ./build ./include/config.h || true

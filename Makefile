
# PYTHON_PREFIX := $(shell brew --prefix python)
# C_INCLUDE="${PYTHON_PREFIX}/Frameworks/Python.framework/Versions/Current/include"
# C_LIB="${PYTHON_PREFIX}/Frameworks/Python.framework/Versions/Current/lib"

# C_FLAGS := $(shell /opt/homebrew/Frameworks/Python.framework/Versions/Current/bin/python3-config --cflags)
# C_FLAGS := $(shell /opt/homebrew/Frameworks/Python.framework/Versions/Current/bin/python3-config --ldflags --embed)
C_FLAGS := $(shell /opt/homebrew/Frameworks/Python.framework/Versions/Current/bin/python3-config --cflags --ldflags --embed)
# C_FLAGS := $(shell /opt/homebrew/Frameworks/Python.framework/Versions/Current/bin/python3-config --include --ldflags --embed)

.PHONY: exec

all: clean exec

build/main:
# @gcc -I${C_INCLUDE} -L${C_LIB} -lpython3.13 -fno-strict-overflow -Wsign-compare -Wunreachable-code -fno-common -dynamic -DNDEBUG -g -O3 -Wall ./c_project/main.c -o ./c_project/main
	@gcc ${C_FLAGS} src/main.c -o build/main

exec: build/main
	./build/main

clean:
	@rm ./build/main || true


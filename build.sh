#!/usr/bin/env bash
set -e

PYTHON_PREFIX="$(brew --prefix python)"
C_INCLUDE="${PYTHON_PREFIX}/Frameworks/Python.framework/Versions/Current/include"
C_LIB="${PYTHON_PREFIX}/Frameworks/Python.framework/Versions/Current/lib"

mkdir -p ./build
gcc -I"${C_INCLUDE}" -L${C_LIB} -lpython3.13 ./src/main.c -o ./build/main
./build/main


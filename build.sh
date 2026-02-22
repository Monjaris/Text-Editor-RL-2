#!/bin/bash

SRC_FILES=$(find -name "*.cpp")
INCLUDE_DIRS="-Iinclude"
BIN_DIR="./bin"

CPP_VERSION="-std=c++20"
OPTIMIZE="-O0"
DEBUG="-g"
FLAGS=" \
-Wall -Wcomment \
"

# Compile all
g++ $SRC_FILES $INCLUDE_DIRS \
$CPP_VERSION $OPTIMIZE $DEBUG $FLAGS \
-lraylib -lGL -lm -lpthread -ldl -lrt -o "$BIN_DIR"/main


iscompiled=$?

# If compiled, run
if [ $iscompiled -eq 0 ]; then
    "$BIN_DIR"/main
fi


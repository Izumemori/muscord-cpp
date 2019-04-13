#!/bin/bash

INSTALL_PATH=`pwd`

echo "Building discord-rpc..."

cd discord-rpc/
mkdir -p build
cd build/
cmake .. -DCMAKE_INSTALL_PREFIX="$INSTALL_PATH"
cmake --build . --config Release --target install

cd "$INSTALL_PATH"

echo "Building muscord..."

mkdir -p build/

PLAYERCTL_FLAGS=`pkg-config --cflags playerctl`
PLAYERCTL_LIBS=`pkg-config --libs playerctl`

g++ -DDEBUG -g $GLIB_FLAGS $PLAYERCTL_FLAGS main.cpp muscord.cpp playerctl.cpp $GLIB_LIBS $PLAYERCTL_LIBS -Llib64/ -ldiscord-rpc -lpthread -o build/muscord
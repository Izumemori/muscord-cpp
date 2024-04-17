#!/bin/bash

DEBUG=$@

DEBUG_OPTION=""

if [ "$DEBUG" == "-d" ]; then
    DEBUG_OPTION="-DDEBUG=ON"
fi

echo "Trying to create directories..."
mkdir -p build
mkdir -p bin

cd build/

echo "Building..."
cmake .. -DCMAKE_INSTALL_PREFIX=. $DEBUG_OPTION
make -j4
cd ..

cp build/bin/muscord ./bin/

echo "Executable can be found in 'bin'."
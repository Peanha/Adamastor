#!/bin/sh

if [ "$1" = "debug" ]; then
    rm -rf build
    cmake --preset debug && cmake --build --preset debug
elif [ "$1" = "" ]; then
    rm -rf build
    cmake --preset default && cmake --build --preset default
fi



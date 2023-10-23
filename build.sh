#!/bin/bash

for param in "$*"
do
    if [[ $param == "--install" ]] ; then
        apt update && apt install libfcgi-dev libglib2.0-dev libcurl4-openssl-dev \
            libjansson-dev cmake clang libglib2.0-dev
    fi
done

cmake .
cmake --build .
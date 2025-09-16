#!/bin/bash

set -e

mkdir -p build/picotool-local

pushd build
    if [ ! -f picotool-local/picotool ]; then
        pushd picotool-local
            export PICO_SDK_PATH=../../lib/pico-sdk
            cmake ../../lib/picotool
            make
        popd
    fi

    export PICO_SDK_PATH=../lib/pico-sdk
    cmake .. -DPICO_BOARD=pico2_w
    make org2-datapack-test
    sudo picotool-local/picotool load -v -x org2-datapack-test.uf2 -f
popd
#!/bin/bash

ROOT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
BUILD_DIR=${ROOT_DIR}/build/linux

cmake \
    -S ${ROOT_DIR} \
    -B ${BUILD_DIR} \
    -G "Unix Makefiles"
    
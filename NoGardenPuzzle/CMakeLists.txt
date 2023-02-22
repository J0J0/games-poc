cmake_minimum_required(VERSION 3.1)
project(NoGardenPuzzle LANGUAGES NONE) # actually: Haskell

set(CABAL_BUILDDIR "${CMAKE_CURRENT_BINARY_DIR}/cabal")
set(CABAL_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/cabal-build-and-copy-binary.sh")

if(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(COPY_TARGET "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
else()
    set(COPY_TARGET "${CMAKE_CURRENT_BINARY_DIR}")
endif()

file(GENERATE
    OUTPUT "${CABAL_SCRIPT}"
    CONTENT
"#!/bin/sh
cd \"${CMAKE_CURRENT_SOURCE_DIR}\"
cabal build --builddir=\"${CABAL_BUILDDIR}\"
cp --target-directory=\"${COPY_TARGET}\" \"$(cabal list-bin --builddir=\"${CABAL_BUILDDIR}\" NoGardenPuzzle)\"
"
)

add_custom_target(NoGardenPuzzle ALL
    COMMAND sh "${CABAL_SCRIPT}"
)

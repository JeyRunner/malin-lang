# Currentlly broken due to missing import error, most likely due to missing llvm-toolchain for libc
FROM alpine:3.13

ARG LLVM=10

# build dependencies
RUN apk add --no-cache build-base dpkg cmake make

# llvm
RUN apk add --no-cache llvm${LLVM} llvm${LLVM}-dev llvm${LLVM}-static llvm${LLVM}-libs

# clang
RUN apk add --no-cache clang clang-dev lldb lld g++ libstdc++

# cmake hunter dependencies
RUN apk add --no-cache git zlib-dev


WORKDIR /home

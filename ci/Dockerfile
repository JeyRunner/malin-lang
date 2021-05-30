FROM ubuntu:20.04

RUN apt update && apt -y install make cmake
RUN apt -y install llvm-10-dev libclang-common-10-dev clang
RUN apt -y install git build-essential zlib1g-dev
#RUN apt -y install llvm-toolchain-10 clang

WORKDIR /home
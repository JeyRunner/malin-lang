FROM ubuntu:focal

ARG LLVM=10
RUN apt update && apt upgrade -y

# build dependencies
RUN apt --no-install-recommends -y install make cmake

# llvm
RUN apt --no-install-recommends -y install llvm-${LLVM}-dev

# clang
RUN apt --no-install-recommends -y install libclang-common-${LLVM}-dev clang

# cmake hunter dependencies
RUN apt -y install git build-essential zlib1g-dev

# c++20 via g++-10
RUN apt -y install software-properties-common &&\
    add-apt-repository -y ppa:ubuntu-toolchain-r/test  &&\
    apt -y install g++-10 libstdc++-10-dev

WORKDIR /home
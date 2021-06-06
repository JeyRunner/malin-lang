FROM debian:buster-slim

ARG LLVM=10
RUN apt update && apt upgrade -y

# build dependencies
RUN apt -y install wget gnupg
RUN apt --no-install-recommends -y install make cmake

# llvm
RUN wget -O - "https://apt.llvm.org/llvm-snapshot.gpg.key" | apt-key add - \
    && echo "deb http://apt.llvm.org/buster/ llvm-toolchain-buster-${LLVM} main \
             \ndeb-src http://apt.llvm.org/buster/ llvm-toolchain-buster-${LLVM} main" \
       | tee /etc/apt/sources.list.d/llvm${LLVM}.list \
    && apt update \
    && apt --no-install-recommends -y install llvm-${LLVM}-dev

# clang
RUN apt --no-install-recommends -y install libclang-common-${LLVM}-dev clang

# cmake hunter dependencies
RUN apt -y install git build-essential zlib1g-dev

WORKDIR /home
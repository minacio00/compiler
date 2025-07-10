# Dockerfile

FROM ubuntu:18.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
  apt-get install -y gcc-6 make libc6-dev && \
  ln -sf /usr/bin/gcc-6 /usr/bin/gcc && \
  rm -rf /var/lib/apt/lists/*

ENV CC=gcc
ENV CFLAGS="-std=c90 -Wall -Wextra -g -Iinclude"

WORKDIR /app

COPY . .

RUN make clean && make

CMD ["./lex"]


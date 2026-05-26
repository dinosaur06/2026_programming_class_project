FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \ 
    build-essential \
    g++ \
    cmake \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY ./2026_programming_class_project /src
RUN g++ -O3 -o game_server *.cpp

EXPOSE 9000/udp

CMD ["./game_server"]
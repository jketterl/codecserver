FROM debian:buster-slim

COPY . /tmp/codecserver

RUN apt-get update && \
    apt-get -y install --no-install-recommends cmake libprotobuf17 libprotobuf-dev protobuf-compiler make gcc g++ && \
    mkdir -p /tmp/codecserver/build && \
    cd /tmp/codecserver/build && \
    cmake -DCMAKE_INSTALL_PREFIX=/usr .. && \
    make && \
    make install && \
    mkdir -p /etc/codecserver && \
    cp ../conf/codecserver.conf /etc/codecserver && \
    cd && \
    apt-get -y purge --autoremove cmake libprotobuf-dev protobuf-compiler make gcc g++ && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* && \
    rm -rf /tmp/codecserver


CMD [ "/usr/bin/codecserver" ]
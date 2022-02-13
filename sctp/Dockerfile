FROM quay.io/centos/centos:stream8
RUN yum install -y make gcc lksctp-tools-devel
RUN yum install -y lksctp-tools

RUN mkdir -p /app/src

COPY src /app/src/

WORKDIR /app/src
RUN make

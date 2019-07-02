FROM alpine

RUN apk add build-base autoconf automake libwebsockets-dev sqlite-dev perl-mojolicious

RUN autoreconf -im

RUN mkdir build && cd build && ../configure

RUN make install

CMD ["muddled"]
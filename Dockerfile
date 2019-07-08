FROM alpine as builder

RUN apk add build-base cmake openssl-dev libwebsockets-dev sqlite-dev lua5.1-libs lua5.1-dev check-dev

WORKDIR /build

COPY . .

RUN cmake -DCMAKE_INSTALL_PREFIX=/dist -DCMAKE_BUILD_TYPE=Release

RUN make install

FROM alpine

RUN apk add ca-certificates openssl-dev libwebsockets-dev sqlite-dev perl-mojolicious lua5.1-libs lua5.1-dev

COPY --from=builder /dist /dist

EXPOSE 4000

CMD ["/dist/sbin/muddled"]


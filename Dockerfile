FROM phusion/baseimage:0.9.16

# Use baseimage-docker's init system.
CMD ["/sbin/my_init"]

RUN apt-get dist-upgrade
RUN apt-get update
RUN apt-get upgrade -y
RUN apt-get --assume-yes install gcc make libtool libsqlite3-dev libc6-dev liblua5.1-dev libwebsockets-dev autoconf automake
RUN mkdir -p /etc/service/muddled /usr/src/muddled
ADD scripts/muddled.sh /etc/service/muddled/run
COPY . /usr/src/muddled
WORKDIR /usr/src/muddled
RUN autoreconf && ./configure --prefix=/usr && make install
RUN make distclean
# Clean up APT when done.
RUN apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
EXPOSE 3778

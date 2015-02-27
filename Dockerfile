FROM phusion/baseimage:0.9.16

# Use baseimage-docker's init system.
CMD ["/sbin/my_init"]

RUN apt-get update
RUN apt-get --assume-yes install gcc make libtool libsqlite3-dev libc6-dev libwebsockets-dev autoconf automake
RUN mkdir /etc/service/muddled
ADD scripts/muddled.sh /etc/service/muddled/run
COPY . /usr/src/muddled
WORKDIR /usr/src/muddled
RUN autoreconf && ./configure && make install
RUN make distclean

# Clean up APT when done.
RUN apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*


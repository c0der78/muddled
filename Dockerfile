FROM phusion/baseimage:0.9.16

# Use baseimage-docker's init system.
CMD ["/sbin/my_init"]

RUN apt-get update
RUN apt-get install gcc gradle
RUN mkdir /etc/service/muddled
ADD scripts/muddled.sh /etc/service/muddled/run
ADD . /usr/src/muddled

RUN gradle -p /usr/src/muddled assemble
RUN cp -f /usr/src/muddled/build/binaries/release/muddled /usr/bin/muddled
RUN gradle -p /usr/src/muddled clean

# Clean up APT when done.
RUN apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*


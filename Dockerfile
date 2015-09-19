FROM phusion/baseimage:0.9.17

# Use baseimage-docker's init system.
CMD ["/sbin/my_init"]

# do some package stuff
RUN apt-get dist-upgrade
RUN apt-get update
RUN apt-get upgrade -y
RUN apt-get --assume-yes install gcc make libtool libsqlite3-dev libc6-dev liblua5.1-dev libwebsockets-dev autoconf automake libnet-ssleay-perl ruby ruby-sass

# install some perl for web
RUN PERL_MM_USE_DEFAULT=1 cpan install URI Mozilla::CA IO::Socket::SSL IO::Socket::IP Mojolicious Mojolicious::Plugin::Authentication Mojolicious::Plugin::AssetPack Mojolicious::Plugin::Bootstrap3 Mojolicious::Plugin::Database

# add some services
RUN mkdir -p /etc/service/muddled /etc/service/muddled-web
ADD scripts/muddled.sh /etc/service/muddled/run
ADD scripts/muddled-web.sh /etc/service/muddled-web/run

# build the game
RUN mkdir -p /usr/src/muddled
COPY . /usr/src/muddled
WORKDIR /usr/src/muddled
RUN autoreconf
RUN ./configure --prefix=/usr
RUN make install

# cleanup build
RUN make distclean

# Clean up APT when done.
RUN apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
EXPOSE 23:23
EXPOSE 24:24
EXPOSE 80:80
EXPOSE 443:443

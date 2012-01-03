#!/bin/sh

PREFIX=./release
BIN=$PREFIX/bin/muddyplains
STOP=$PREFIX/var/log/shutdown.log

if [ "X$1" != "X" ];then
	kill -9 $1 #kill previous instance of script
fi

if [ -e $STOP ]; then
	echo "Please remove $STOP first."
	exit
fi

if ! `pidof $BIN` > /dev/null ; then
	$BIN -p 3778 >> /dev/null &
fi
sleep 300 #time in seconds for recheck
#record script pid for next kill and start another #instance
pid=$$
$0 $pid

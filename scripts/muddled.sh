#!/bin/sh
echo "Starting muddled..."
exec /usr/bin/muddled -p 23 >>/var/log/muddled.log 2>&1

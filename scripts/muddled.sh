#!/bin/sh
echo "Starting muddled..."
exec /usr/bin/muddled -p 3778 >>/var/log/muddled.log 2>&1

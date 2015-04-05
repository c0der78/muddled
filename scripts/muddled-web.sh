#!/bin/sh
echo "Starting muddled web..."
exec morbo -m production -l https://*:443 -l http://[::]:80 /usr/var/www/muddled.pl

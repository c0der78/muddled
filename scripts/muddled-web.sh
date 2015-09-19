#!/bin/sh
MOJO_HOME=/usr/var/www 
MOJO_MODE=production
chdir $MOJO_HOME
echo "Starting muddled web..."
exec hypnotoad -f script/muddled
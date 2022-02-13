#!/bin/sh

set -x

mkfifo flag

trap 'rm -f flag' EXIT

make || exit 1

modprobe sctp || exit 1

./sctp_bz2048251_server -f flag -4 9999 &
read <>flag
./sctp_bz2048251_client 127.0.0.1 9999

wait

echo "Reproducer completed - check for AVC denials (ausearch -i -m avc -ts recent)"

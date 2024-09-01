#!/bin/sh
for i in $(seq 1 31) ; do
	echo "./silly -$i" > test-signal"$i".sh
done

rm test-signal18.sh	# SIGCONT
rm test-signal19.sh	# SIGSTOP
rm test-signal20.sh # SIGTSTP
rm test-signal21.sh # SIGTTIN
rm test-signal22.sh # SIGTTOU


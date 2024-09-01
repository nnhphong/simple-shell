#!/bin/sh
for i in $(seq 0 15) ; do
	echo "./silly $i" > test-child-exit"$i".sh
done



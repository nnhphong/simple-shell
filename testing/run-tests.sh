#!/bin/sh

./test-signals.sh
./test-child-exits.sh

chmod u+x test*.sh

failed=''

echo "Testing for echo..."
./automatic tests/test-1.sh 0

if [ $? -eq 1 ] ; then failed="$failed""test-1.sh " ; fi

echo "Testing for forx..."
for i in $(seq 2 7) ; do
	./automatic tests/test-$i.sh 0
	if [ $? -eq 1 ] ; then failed="$failed""test-$i.sh " ; fi
done

echo "Testing for list..."
./automatic tests/test-8.sh 1
if [ $? -eq 1 ] ; then failed="$failed""test-8.sh " ; fi
./automatic tests/test-9.sh 0 
if [ $? -eq 1 ] ; then failed="$failed""test-9.sh " ; fi
./automatic tests/test-10.sh 0
if [ $? -eq 1 ] ; then failed="$failed""test-10.sh " ; fi
./automatic tests/test-11.sh 6
if [ $? -eq 1 ] ; then failed="$failed""test-11.sh " ; fi
./automatic tests/test-12.sh 1
if [ $? -eq 1 ] ; then failed="$failed""test-12.sh " ; fi
./automatic tests/test-13.sh 2
if [ $? -eq 1 ] ; then failed="$failed""test-13.sh " ; fi
./automatic tests/test-14.sh 4
if [ $? -eq 1 ] ; then failed="$failed""test-14.sh " ; fi
echo "Testing for signals..."
for i in $(seq 1 17) ; do
	./automatic tests/test-signal$i.sh 0
if [ $? -eq 1 ] ; then failed="$failed""test-signal$i.sh " ; fi
done
for i in $(seq 23 31) ; do
	./automatic tests/test-signal$i.sh 0
if [ $? -eq 1 ] ; then failed="$failed""test-signal$i.sh " ; fi
done

echo "Testing for child exits..."
for i in $(seq 0 15) ; do
	./automatic tests/test-child-exit$i.sh 0
if [ $? -eq 1 ] ; then failed="$failed""test-child-exit$i.sh " ; fi
done

echo "Failed Cases"
echo "$failed"

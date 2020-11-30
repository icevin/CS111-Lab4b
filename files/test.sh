#!/bin/bash

# NAME: Kevin Tang
# EMAIL: kevintang2023@ucla.edu
# ID: 805419480

counter=0

./lab4b --BOGUS &> /dev/null
if [[ $? -ne 1 ]]
then
	echo "FAIL: bogus argument test"
else
	echo "PASS: bogus argument"
	((counter=counter+1))
fi

echo "Testing program execution..."

{ sleep 2; echo "SCALE=C"; sleep 2; echo "PERIOD=2"; sleep 1; echo "OFF"; } | ./lab4b --log=log.test > /dev/null

if [[ $? -ne 0 ]]
then
	echo "FAIL: Program exited unsuccessfully"
else
	echo "PASS: Ran successfully!"
	((counter=counter+1))
fi

echo "PASSED ${counter} OUT OF 2 CASES"

rm -f log.test
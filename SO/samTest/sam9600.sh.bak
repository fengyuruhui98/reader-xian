#!/bin/sh
export LD_LIBRARY_PATH=/hhjt/reader:$LD_LIBRARY_PATH
chmod 777 samTest
baud=9600
#echo "please insert sam in SAM Index in 10 seconds"

echo "Test sam index 0 baud " $baud ". no pps" 
./samTest "$j" "$baud"

echo "Test sam index 1 baud" $baud ". no pps"
sleep 10
./samTest 1 "$baud"

echo "Test sam index 2 baud" $baud ". no pps"
sleep 10
./samTest 2 "$baud"

echo "Test sam index 3 baud" $baud ". no pps"
sleep 10
./samTest 3 "$baud"

echo "Test sam index 4 baud" $baud ". no pps"
sleep 10
./samTest 4 "$baud"

echo "Test sam index 5 baud" $baud ". no pps"
sleep 10
./samTest 5 "$baud"

echo "Test sam index 6 baud" $baud ".no pps"
sleep 10
./samTest 6 "$baud"

echo "Test sam index 7 baud" $baud ". no pps"
sleep 10
./samTest 7 "$baud"
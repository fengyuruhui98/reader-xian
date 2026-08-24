export LD_LIBRARY_PATH=/mnt/yaffs/xian:$LD_LIBRARY_PATH
if [ -f ./progbak/suzhou ]
then
	echo "\nprogram error! now back to the old version..."
	mv ./progbak/suzhou .
else
	if [ -f ./prognew/update ]
	then
		echo "now update the reader application..."
		rm ./prognew/update
		cp suzhou ./progbak/.
		mv ./prognew/suzhou .
	fi
fi
echo "run the reader application...."
chmod 700 * -R
./suzhou /dev/ttySAC0

###first arg = interface 
###second arg = output file 
###third arg = buffer file to store wget's output
echo --------------- wlan0 ------------- | tee wifi_bw/wifi_debug
ipaddress=`ifconfig $1 | grep inet | awk '{print $2}' | head -1`

../../useful-tools/wget --bind-address $ipaddress -T 5 -t 1 -o wifi_bw/$3 202.141.1.54 

num_of_line=`grep "Throughput" wifi_bw/$3 | wc -l`
#echo $num_of_line

if [ $num_of_line == 0 ];then
	echo 0 >> wifi_bw/$2 | tee wifi_bw/wifi_debug
	echo "wget failed" | tee wifi_bw/wifi_debug
	rm -r index.html wifi_bw/$3
else
#	val=`grep "index.html" threeg_bw/$3 | awk -F"(" '{print $2}'|awk -F")" '{print $1}' | awk -F" " '{print $1}'`
	val=`grep "Throughput" wifi_bw/$3 | awk -F" " '{print $2}'`
	unit=`grep "Throughput" wifi_bw/$3 | awk -F" " '{print $3}'`
	if [ "$unit" = "MB/s" ];then
		
		k=1000
		echo before $val | tee wifi_bw/wifi_debug 
		Number=`echo 1000 \* $val |bc`
		echo $Number >> wifi_bw/$2
	else
	echo $val >> wifi_bw/$2
	echo $val >> wifi_bw/wifi_debug
	fi
	rm -rf index.html* wifi_bw/$3
fi

#cat threeg_bw/$2

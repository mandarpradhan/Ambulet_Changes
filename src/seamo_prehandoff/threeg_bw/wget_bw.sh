###first arg = interface 
###second arg = output file 
###third arg = buffer file to store wget's output

echo ----------- $1 ----------- >> threeg_bw/threeg_debug

ipaddress=`ifconfig $1 | grep inet | awk '{print $2}' | head -1`

../../useful-tools/wget --bind-address $ipaddress -T 5 -t 1 -o threeg_bw/$3 202.141.1.54 

num_of_line=`grep "Throughput" threeg_bw/$3 | wc -l`
#echo $num_of_line

if [ $num_of_line == 0 ];then
	echo 0 >> threeg_bw/$2
	echo Download did not complete >> threeg_bw/threeg_debug
	echo "wget failed"
	rm -r index.html threeg_bw/$3
else
	grpval=`grep "Throughput" threeg_bw/$3`
	echo $grpval  >> threeg_bw/threeg_debug
	val=`grep "Throughput" threeg_bw/$3 | awk -F" " '{print $2}'`
	echo $val >> threeg_bw/$2
	echo $val >> threeg_bw/threeg_debug 
	rm -rf index.html* threeg_bw/$3
fi

#cat threeg_bw/$2

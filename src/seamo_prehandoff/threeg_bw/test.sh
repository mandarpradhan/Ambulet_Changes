###first arg = interface 
###second arg = output file 
###third arg = buffer file to store wget's output

ipaddress=`ifconfig $1 | grep inet | awk '{print $2}' | head -1`

wget --bind-address $ipaddress -T 3 -t 1 -o threeg_bw/$3 10.20.10.10 

num_of_line=`grep "index.html" threeg_bw/$3 | wc -l`
#echo $num_of_line

if [ $num_of_line == 0 ];then
	echo 0 > threeg_bw/$2
	echo "wget failed"
else
	val=`grep "index.html" threeg_bw/$3 | awk -F"(" '{print $2}'|awk -F")" '{print $1}' | awk -F" " '{print $1}'`
	echo $val > threeg_bw/$2
	rm -r index.html threeg_bw/$3
fi

cat threeg_bw/$2

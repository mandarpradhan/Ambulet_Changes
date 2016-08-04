

echo $1 $2
#ping -I $1 -f -s 1024 -c 5 202.141.1.20 | grep rtt | awk -F/ '{print $5}' > threeg_bw/$2
val=`ping -I $1 -f -s 1024 -c 5 202.141.1.20 | grep rtt | awk -F/ '{print $5}'`
echo $val > threeg_bw/$2
cat threeg_bw/$2

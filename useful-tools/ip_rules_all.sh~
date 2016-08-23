# move ip_rules_all.sh
# read before execution HOWTO_CONFIGURE_3G_DEVICE

#! /bin/bash
########## ppp0 ###############
is_iface=`ifconfig | grep ppp0 | wc -l`
if [ $is_iface == 0 ];then
	echo ppp0 not available
else
	gateway_ppp0=`ifconfig  ppp0 | grep p-t-2 | awk '{print $3}'`
	echo $gateway_ppp0

	ipaddress_ppp0=`ifconfig ppp0 | grep inet | awk '{print $2}'`
	echo $ipaddress_ppp0

	ip route replace default via $gateway_ppp0 table gw1

	ip rule add from $ipaddress_ppp0 table gw1
fi
################ ppp1 ##############

is_iface=`ifconfig | grep ppp1 | wc -l`
if [ $is_iface == 0 ];then
	echo ppp1 not available
else
	gateway_ppp1=`ifconfig  ppp1 | grep destination | awk '{print $6}'`
	echo $gateway_ppp1

	ipaddress_ppp1=`ifconfig ppp1 | grep inet | awk '{print $2}'`
	echo $ipaddress_ppp1

	ip route replace default via $gateway_ppp1 table gw2

	ip rule add from $ipaddress_ppp1 table gw2
fi

################ ppp2 ##############

is_iface=`ifconfig | grep ppp2 | wc -l`
if [ $is_iface == 0 ];then
	echo ppp2 not available
else
	gateway_ppp2=`ifconfig  ppp2 | grep destination | awk '{print $6}'`
	echo $gateway_ppp2

	ipaddress_ppp2=`ifconfig ppp2 | grep inet | awk '{print $2}'`
	echo $ipaddress_ppp2

	ip route replace default via $gateway_ppp2 table gw4

	ip rule add from $ipaddress_ppp2 table gw4
fi
############ wifi #################

is_iface=`ifconfig | grep wlan0 | wc -l`
if [ $is_iface == 0 ];then
	echo wlan0 not available
else
	gateway_wlan0=202.41.124.33 
	echo $gateway_wlan0

	ipaddress_wlan0=`ifconfig wlan0 | grep inet | awk '{print $2}' | head -1`
	echo $ipaddress_wlan0

	ip route replace default via $gateway_wlan0 table gw3

	ip rule add from $ipaddress_wlan0 table gw3
fi

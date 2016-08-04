#! /bin/bash

#read -p 'enter ip interface :' ip_interface

ip_interface=$1
echo $ip_interface
gateway_ppp0=`ifconfig $ip_interface | grep destination | awk '{print $6}'`
echo $gateway_ppp0

ipaddress_ppp0=`ifconfig $ip_interface | grep inet | awk '{print $2}'`
echo $ipaddress_ppp0

ip route add default via $gateway_ppp0 table gw1

ip rule add from $ipaddress_ppp0 table gw1

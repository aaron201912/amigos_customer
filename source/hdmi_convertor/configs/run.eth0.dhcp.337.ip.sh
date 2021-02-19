#!/bin/sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/lib:`pwd`/lib
export PATH=$PATH:`pwd`/bin
echo $LD_LIBRARY_PATH
echo $PATH
mkdir -p /var/wifi/misc
mkdir -p /var/run/hostapd/
rm -f /dev/random
ln -s /dev/urandom /dev/random
#ifconfig p2p0 up
#iwpriv p2p0 fwcmd set_freq,4,2380
#ifconfig p2p0 192.168.1.101 netmask 255.255.255.0
#hostapd -B ./hostapd.conf
#sleep 3
#dnsmasq -i p2p0 --no-daemon -C ./dnsmasq.conf &
#./kvm_dev &
#-------------------------------------------------
ifconfig eth0 up
ifconfig eth0 172.19.24.11  netmask 255.255.255.0
ifconfig eth0 hw ether 00:70:33:00:00:01
#ifconfig eth0 hw ether 00:70:39:00:00:01; ifconfig eth0 172.19.25.224 netmask 255.255.255.0;route add default gw 172.19.25.254
telnetd &
#dnsmasq -i eth0 --no-daemon -C ./dnsmasq.conf &
#./dev &
./hdmi_convertor 2M_AUDIO.ini
#echo 4 > /proc/mi_modules/mi_vpe/debug_level
#mount -t nfs -o nolock 172.19.25.28:/e/formVdi  /mnt
#echo 0 > /proc/sys/kernel/printk
#cd customer/hdmiap/

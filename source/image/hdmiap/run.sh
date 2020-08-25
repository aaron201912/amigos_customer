#!/bin/sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/lib:`pwd`/lib
export PATH=$PATH:`pwd`/bin
echo $LD_LIBRARY_PATH
echo $PATH
mkdir -p /var/wifi/misc
mkdir -p /var/run/hostapd/
rm -f /dev/random
ln -s /dev/urandom /dev/random
ifconfig p2p0 up
iwpriv p2p0 fwcmd set_freq,4,2380
ifconfig p2p0 192.168.1.101 netmask 255.255.255.0
hostapd -B ./hostapd.conf
sleep 3
dnsmasq -i p2p0 --no-daemon -C ./dnsmasq.conf &
./hdmi_convertor 2M_AUDIO.ini

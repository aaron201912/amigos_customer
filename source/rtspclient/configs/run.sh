#!/bin/sh
#run by 201/202/203
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/lib:`pwd`/lib
export PATH=$PATH:`pwd`/bin
echo $LD_LIBRARY_PATH
echo $PATH
mkdir -p /tmp/wifi/run
chmod 777 /tmp/wifi/run
rm -f /dev/random
ln -s /dev/urandom /dev/random
if [ -z $(lsmod |grep ssw101b_wifi_HT40_usb) ];then
insmod /config/wifi/ssw101b_wifi_HT40_usb.ko
fi
ifconfig wlan0 up
iwpriv wlan0 fwcmd set_freq,4,2380
wpa_supplicant -Dnl80211 -i wlan0 -c ./wpa_supplicant.conf &
echo "finish connect ap....."
udhcpc -q -i wlan0 -s /etc/init.d/udhcpc.script
./rtspclient rtsp://192.168.1.100/main_stream  RTSP_CLIENT_HDMI.ini   1920 1080

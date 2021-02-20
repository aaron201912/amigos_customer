#!/bin/sh
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/lib:`pwd`/lib
export PATH=$PATH:`pwd`/bin
echo $LD_LIBRARY_PATH
echo $PATH
ifconfig eth0 up
ifconfig eth0 172.19.24.201  netmask 255.255.255.0
ifconfig eth0 hw ether 00:70:33:00:00:02

if [ "`cat /customer/demo.sh | grep drop_frame`" = "" ]; then
	sed -i '/mi_disp.ko/ s/$/ drop_frame=1/' /customer/demo.sh;
fi

./rtspclient rtsp://172.19.24.11/main_stream  RTSP_CLIENT_HDMI.ini   1920 1080
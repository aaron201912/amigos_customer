# AMIGOS
Amigos这一套软件系统为了达到尽量少改动代码的目的，把mi模块串接和设定的流程尽量在INI中配置的一套软件架构。它参考MI的模块划分，也有VPE、DIVP、VENC等模块，同时也有类似的SYS模块，负责模块与模块之间的串接以及数据通讯。除此之外，Amigos也有一些新创建的模块，处理流程与mi底层类似，例如RTSP模块负责接收和发送ES/PCM流。例如FILE模块，可以配置输入和输出，不用改code，就可以存储前端模块发送过来的数据，或者读取文件发送给后端模块。
## 编译执行和打包方法:
### 1.编译：
进入到
> cd amigos/source/

> declare -x ALKAID_PATH=xxxx

> make clean&make

编译完成后输入 make install 会在 release/ 下打包成文件夹和squashfs
如果ALIAID的project是337de的，则编译的amigos是source端的，如果ALIAID的project是202/203/msr650的，则编
译出来的amigos是Sink端的。
### 2.执行：
#### Source端:
bin:
> amigos_customer/source/out/app/arm/hdmi_convertor

OSD位图资源文件:
+ 显示英文 "NO SIGNAL":
> amigos_customer/source/hdmi_convertor/config/nosignal.index2

+ 显示中文 "无信号":
> amigos_customer/source/hdmi_convertor/config/wuxinhao.index2

> INI中 OSD_SRC_FILE=./xxx.index2可以配置使用位图源文件，格式为I2.

在板子上执行：
> ./hdmi_convertor ./2M_AUDIO.ini

2M_AUDIO.ini所在位置：

+ 338/339G/8629使用

> source/hdmi_convertor/config/pudding/2M_AUDIO.ini

+ 337de使用

> source/hdmi_convertor/config/ispahan/2M_AUDIO.ini

#### Sink端：

##### 应用 rtspclient:

应用运行环境:
+ amigos_cusomer/release/hdmiplayer

bin:

+ amigos_customer/source/out/app/arm/rtspclient

功能:

+ 播放一路码流

config:

+ 202使用
++ amigos_customer/source/rtspclient/config/RTSP_CLIENT_DISP.ini

+ 203使用
++ amigos_customer/source/rtspclient/config/RTSP_CLIENT_HDMI.ini

+  msr650以hdmitx输出video使用:
++ amigos_customer/source/rtspclient/config/RTSP_CLIENT_DIVP_HDMI.ini

在板子上执行:

+ 202使用 ./rtspclient rtsp://xxx.xxx.xxx.xxx RTSP_CLIENT_DISP.ini 1024 600
> 注：1024 600是屏的宽高，根据具体情况去配置

+ 203使用 ./rtspclient rtsp://xxx.xxx.xxx.xxx RTSP_CLIENT_HDMI.ini 1920 1080
> 注：hdmi默认输出1080p60

+ msr650使用 ./rtspclient rtsp://xxx.xxx.xxx.xxx RTSP_CLIENT_DIVP_HDMI.ini 1920 1080
> 注：hdmi默认输出1080p60

##### 应用 preview:

bin:

amigos_customer/source/out/app/arm/preview

功能:

根据不同的ini配置可播放一路或多路码流，次应用相当于一个全功能版本，可以任意配置ini达到串流的目的，202、203也可使用此应用，但是目前使用rtspclient已经满足需求。

config:

+ msr650以hdmitx输出一路video:
> amigos_customer/source/preview/config/naboo/RTSP_CLIENT_DIVP_HDMI.ini

+ msr650以hdmitx输出四路video (默认一路rtsp接收，并发4路video显示，可配置4路rtsp接收并发4路video显示):
> amigos_customer/source/preview/config/naboo/RTSP_CLIENT_DIVP_DISP_HDMI_4WIN.ini

msr650在板子上执行：

> ./preview RTSP_CLIENT_DIVP_HDMI.ini

> ./preview RTSP_CLIENT_DIVP_DISP_HDMI_4WIN.ini

## 3rdparty代码编译方法：
> cd amigos_customer/3rdparty

> git submodule init

> git submodule update

> cd amigos_customer/source/

根据依赖全部编译：

> make 3rdparty

单独编译某个第三方代码：

make xxx

例如编译wpa:

wpa 依赖于libnl和openssl，先编译

然后再编译wpa：make wpa_supplicant_3rd_party_all

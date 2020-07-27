# AMIGOS
Amigos这一套软件系统为了达到尽量少改动代码的目的，把mi模块串接和设定的流程尽量在INI中配置的一套软件架构。
它参考MI的模块划分，也有VPE、DIVP、VENC等模块，同时也有类似的SYS模块，负责模块与模块之间的串接以及数据通讯。除此之外，Amigos也有一些新创建的模块，处理流程与mi底层类似，例如RTSP模块负责接收和发送ES/PCM流。例如FILE模块，可以配置输入和输出，不用改code，就可以存储前端模块发送过来的数据，或者读取文件发送给后端模块。

编译方法:

	1. 先编译ALKAID然后再build amigos_customer代码，修改amigos_customer/source/Makefile中DB_ALKAID_PROJ指定ALKAID project 对应current.configs的位置
	2. 如果ALIAID的project是337de的，则编译的amigos是source端的，如果ALIAID的project是202/203的，则编译出来的amigos是Sink端的。
	3. 执行：
	Source端：
		bin: amigos_customer/source/out/app/hdmi_convertor
		config: amigos_customer/source/hdmi_convertor/config/2M_AUDIO.ini
		在板子上执行： ./hdmi_convertor ./2M_AUDIO.ini

	Sink端：
		bin: amigos_customer/source/out/app/rtspclient
		config: 
			--->202使用 amigos_customer/source/rtspclient/config/RTSP_CLIENT_DISP.ini
			--->203使用 amigos_customer/source/rtspclient/config/RTSP_CLIENT_HDMI.ini
		在板子上执行:
			--->202使用 ./rtspclient rtsp://xxx.xxx.xxx.xxx/main_stream RTSP_CLIENT_DISP.ini 1024 600 
				--->注：1024 600是屏的宽高，根据具体情况去配置
			--->203使用 ./rtspclient rtsp://xxx.xxx.xxx.xxx/main_stream RTSP_CLIENT_HDMI.ini 1920 1080 
				--->注：hdmi默认输出1080p60

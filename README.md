# ARM嵌入式设计Linux代码

课程代码 B0264041C

## 使用方法

由于试验箱的CPU架构限制，请在x86 Linux下交叉编译或直接在ARM Linux下编译。

由于试验箱的glibc版本较低，在本地编译时请使用**Ubuntu 20.04及以下环境**。

需要安装的依赖：
- build-essential
- cmake 3.26及以上版本
- boost
- SQLite3

```Shell
cd NJUPT-Embedded-Linux-Design
mkdir build-linux
cmake -B build-linux
cmake --build build-linux
```

然后把build-linux目录下的`program_design`上传至试验箱的Linux内，运行

```Shell
./program_design
```

即可看到软件输出。

## main分支

### 任务

(1)	BlueTooth无线通信模块将加速度传感器采集到的传感数据,上传到BlueTooth网关;

(2)	ARM系统负责接收协调器接收到的传感数据,转换数据格式并保存sqllite;同时通过wifi将数据传送给PC

(3)	在PC上通过socket接收数据，并实现可视化界面（C#、VC++、Web、QT等）,给出多种形式的统计结果(图表)。

### 完成的内容

厂家没有给六合一传感器的单片机驱动代码，所以无法在单片机上驱动传感器。

征询老师后直接使用Linux读取传感器数据，并使用WebSocket传输至Web端。

### Zigbee分支

### 任务

题目23

(1)	Zigbee无线通信模块将温湿度传感器采集到的温湿度数据,上传到Zigbee协调器;

(2)	ARM系统负责接收协调器接收到的温湿度数据,转换数据格式并保存sqllite;

(3)	在ARM系统里利用BOA搭建web服务器，并在web网页上实现可视化界面,给出多种形式的统计结果(图表)，在PC上通过局域网访问网页。

### 完成的内容

使用Zigbee与传感器向连接，需要注意的是，在Linux的Zigbee软件中设计无线通道和PANID后，单片机内需要保证这两个参数一致，其使用完后需要重启整个机器。

获取数据后将数据存入SQLite。

## Bluetooth分支

### 任务

题目26

(1)	BlueTooth无线通信模块将光敏传感器采集到的光传感数据,上传到BlueTooth网关;

(2)	ARM系统负责接收协调器接收到的光传感数据,转换数据格式并保存sqllite;同时通过wifi将数据传送给PC

(3)	利用Linux QT开发ARM系统的数据可视化界面,并给出多种形式的统计结果(图表),显示在实验箱液晶屏

### 完成的内容

使用蓝牙读取传感器数据，需要注意的是读取传感器数据时必须要等待一定的时间，不然会卡住。

同时，发现无法建立连接时需要reset单片机和Linux的蓝牙模块。

获取数据后使用WebSocket传输至PC客户端。

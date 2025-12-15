# FastDDS环境配置

## 1.依赖安装

安装GCC、G++：版本9.4.0，更高的应该也可以，不行再换。`openjdk-8-jdk`后续如果版本较低报错也需要更换为高版本。

```shell
sudo apt install gcc-9 g++-9
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 90
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 90
sudo apt install python3-colcon-common-extensions python3-vcstool zip openjdk-8-jdk  -y
sudo apt-get install -y libasio-dev libtinyxml2-dev
```

## 2.foonathan_memory安装

> 安装foonathan_memory，版本为0.7.3（所有安装均在~/Fast-DDS下）

```shell
cd ~/Fast-DDS
git clone https://github.com/eProsima/foonathan_memory_vendor.git
# 不能生成动态库就用这个，或者看下面的解决办法
# git clone https://github.com/foonathan/memory.git
cd foonathan_memory_vendor
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=~/Fast-DDS/install -DBUILD_SHARED_LIBS=ON
cmake --build . --target install
```

如果未能生成动态库可能是因为安装了ROS2的原因，具体如下：

> 这里是`cmake`找到了`Foonathan memory`这个库，从而不会去编译源码，导致安装失败，原因在于如果你的主机里安装了`ROS2`，`ROS2`里有`Foonathan memory`相关的`cmake`文件，而`ROS2`里的这个库实际上是个空壳子。为了解决这个问题，只需要在编译`Foonathan memory`时先去把`ROS2`的环境变量注释了，一般来说是在`.bashrc`中，把下面的环境变量注释即可，编译完成后再解除注释(ROS的环境变量)
>
> ```bash
> source /opt/ros/galactic/setup.bash
> ```

## 3.安装FastCDR

> 安装FastCDR，版本为v1.0.15

```shell
cd ~/Fast-DDS
git clone https://github.com/eProsima/Fast-CDR.git
cd Fast-CDR/
git checkout v1.0.15
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=~/Fast-DDS/install
cmake --build . --target install
```

## 4.安装FastDDS

> 安装FASTDDS，版本为v2.3.0，实质上是FastRTPS

```shell
cd ~/Fast-DDS
git clone --recursive https://github.com/eProsima/Fast-DDS.git
# 那个慢就试试这个，或者换网
# git clone https://github.com/eProsima/Fast-DDS.git
cd Fast-DDS/
git tag
git checkout v2.3.0
mkdir build && cd build
cmake ..  -DCMAKE_INSTALL_PREFIX=~/Fast-DDS/install
cmake --build . --target install
```

## 5.安装FastDDS-Gen

> 安装Fast-DDS-Gen，版本为v2.4.0

```shell
cd ~/Fast-DDS
git clone https://github.com/eProsima/Fast-DDS-Gen.git
cd Fast-DDS-Gen
git checkout v2.4.0
# 下载不了就直接浏览器下载，然后更改配置文件，见下一段
wget https://services.gradle.org/distributions/gradle-7.6.1-bin.zip 
./gradlew assemble
source ~/.bashrc
# fastddsgen idl路径
# 例如：fastddsgen /home/me/Fast-DDS/Fast-DDS-Gen/scripts/HelloWorld.idl
```

> 打开文件夹路径下的`gradle\wrapper\gradle-wrapper.properties`
> 
> 修改distributionUrl为gradle文件夹所在的本地路径，如图所示：

```shell

cd ~/Fast-DDS/Fast-DDS-Gen
vi gradle\wrapper\gradle-wrapper.properties
# 打开后内容如下，修改distributionUrl的值，为你下载的zip的路径
distributionBase=GRADLE_USER_HOME
distributionPath=wrapper/dists
distributionUrl=file:/home/me/Fast-DDS/Fast-DDS-Gen/gradle-7.6.1-bin.zip
zipStoreBase=GRADLE_USER_HOME
zipStorePath=wrapper/dists
```

## 6.设置环境变量

由于环境配置中所有的安装均在~/FastDDS/install文件中，因此需要添加环境变量，否则会找不到相应的库、头文件。
> 在.bashrc文件中导入源

```shell
export PATH=$PATH:/home/me/Fast-DDS/install/bin/  >> ~/.bashrc
export LD_LIBRARY_PATH=~/Fast-DDS/install/lib:$LD_LIBRARY_PATH >> ~/.bashrc
echo 'export PATH=$PATH:/home/me/Fast-DDS/Fast-DDS-Gen/scripts/' >> ~/.bashrc
```

## 7.HelloWorld

在`~/Fast-DDS/Fast-DDS/examples/C++/`文件夹中存在许多测试样例，在`HelloWorldExample`文件夹中进行测试

```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=~/Fast-DDS/install/
./HelloWorldExample publisher
# 开启另一个终端，或者同一局域网的其他主机，执行订阅端
./HelloWorldExample subscriber
```

结果如下：

```bash
me@me-use:~/HelloWorldExample/build$ ./HelloWorldExample subscriber
Starting
Subscriber running. Please press enter to stop the Subscriber
Subscriber matched
Message HelloWorld 1 RECEIVED
Message HelloWorld 2 RECEIVED
Message HelloWorld 3 RECEIVED
Message HelloWorld 4 RECEIVED
Message HelloWorld 5 RECEIVED
Message HelloWorld 6 RECEIVED
Message HelloWorld 7 RECEIVED
Message HelloWorld 8 RECEIVED
Message HelloWorld 9 RECEIVED
Message HelloWorld 10 RECEIVED
Subscriber unmatched

# 发布端
me@me-use:~/HelloWorldExample/build$ ./HelloWorldExample publisher
Starting
Publisher running 10 samples.
Publisher matched
Message: HelloWorld with index: 1 SENT
Message: HelloWorld with index: 2 SENT
Message: HelloWorld with index: 3 SENT
Message: HelloWorld with index: 4 SENT
Message: HelloWorld with index: 5 SENT
Message: HelloWorld with index: 6 SENT
Message: HelloWorld with index: 7 SENT
Message: HelloWorld with index: 8 SENT
Message: HelloWorld with index: 9 SENT
Message: HelloWorld with index: 10 SENT
```

## 8.其他工作


> 如果CMake版本不够高，需要手动高版本CMake，安装CMake3.4.3

```shell
sudo apt-get install -y libssl-dev libcurl4-openssl-dev
wget https://cmake.org/files/v3.4/cmake-3.4.3.tar.gz
tar -zxvf cmake-3.4.3.tar.gz
cd cmake-3.4.3
./bootstrap
make
sudo make install
cmake --version
```

> 因为使用的是Fastrtps，因此在git后需要切换版本，查看当前版本指令如下

```shell
git log --oneline -n 1
```

> 之前CMake装错了，想删除cmake相关的东西，不晓得怎么卸载就搞这个吧

```shell
sudo rm -rf /usr/bin/cmake*
sudo rm -rf /usr/local/bin/cmake*
sudo rm -rf /usr/share/cmake*
```

> 一些依赖包

```shell
sudo apt-get install libasio-dev -y
sudo apt-get install libtinyxml2-dev
```

> 在编译Fast-DDS-Gen时报错，最后发现是jdk版本太低，更换高版本即可，我是更换成17了

```bash
me@me-use:~/Fast-DDS/Fast-DDS-Gen$ ./gradlew assemble

> Task :submodulesUpdate
子模组 'thirdparty/idl-parser'（https://github.com/eProsima/IDL-Parser.git）已对路径 'thirdparty/idl-parser' 注册
正克隆到 '/home/me/Fast-DDS/Fast-DDS-Gen/thirdparty/idl-parser'...
子模组路径 'thirdparty/idl-parser'：检出 '509cfa3b34814ecf780a6b1fd7e680eaa07cedaf'

> Task :idl-parser:compileJava FAILED
/home/me/Fast-DDS/Fast-DDS-Gen/thirdparty/idl-parser/src/main/java/com/eprosima/idl/context/Context.java:1268: 错误: 无法访问org.openjdk.nashorn.api.scripting.NashornScriptEngineFactory
            return new org.openjdk.nashorn.api.scripting.NashornScriptEngineFactory()
                                                        ^
  错误的类文件: /home/me/.gradle/caches/modules-2/files-2.1/org.openjdk.nashorn/nashorn-core/15.4/f67f5ffaa5f5130cf6fb9b133da00c7df3b532a5/nashorn-core-15.4.jar(org/openjdk/nashorn/api/scripting/NashornScriptEngineFactory.class)
    类文件具有错误的版本 55.0, 应为 52.0
    请删除该文件或确保该文件位于正确的类路径子目录中。
1 个错误

> Task :buildIDLParser FAILED

FAILURE: Build failed with an exception.

* What went wrong:
Execution failed for task ':idl-parser:compileJava'.
> Compilation failed; see the compiler error output for details.

* Try:
> Run with --stacktrace option to get the stack trace.
> Run with --info or --debug option to get more log output.
> Run with --scan to get full insights.

* Get more help at https://help.gradle.org

BUILD FAILED in 3s
2 actionable tasks: 2 executed
```


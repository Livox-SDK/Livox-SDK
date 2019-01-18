# Introduction

Livox SDK is the software development kit designed for all Livox products. It is developed based on C/C++ following Livox SDK Communication Protocol, and provides easy-to-use C style API. With Livox SDK, users can quickly connect to Livox products and receive point cloud data. 

Livox SDK consists of Livox SDK communication protocol, Livox SDK core, Livox SDK API, Linux sample, and ROS demo. 

# Livox SDK Communication Protocol

Livox SDK communication protocol opens to all users. It is the communication protocol between user programs and Livox products. The protocol consists of control commands and data format. Please refer to the [Livox SDK Communication Protocol](https://www.livoxtech.com/sdk/downloads) for detailed information.

# Livox SDK Core

Livox SDK provides the implementation of control commands and point cloud data transmission, as well as the C/C++ API. The basic structure of Livox SDK core is shown as below:

![sdk](doc/images/sdk.png)

User Datagram Protocol (UDP) is used for communication between Livox SDK and LiDAR sensors. Please refer to the Livox SDK Communication Protocol for further information. Point cloud data handler supports point cloud data transmission, while command handler receives and sends control commands. And the C/C++ API is based on command handler and point cloud data handler.

The Livox LiDAR sensors can be connected to host directly or through the Livox Hub. Livox SDK supports both connection methods. When LiDAR units are connected to host directly, the host will establish communication with each LiDAR unit individually. And if the LiDAR units connect to host through Hub, then the host only communicates with the Livox Hub while the Hub communicates with each LiDAR unit.

# Livox SDK API

Livox SDK API provides a set of C style functions which can be conveniently integrated in C/C++ programs. Please refer to the [Livox SDK API Reference](https://www.livoxtech.com/sdk/downloads) for detailed information.

## Build Notes

### Dependencies

LivoxTech SDK requires cmake 3.0.0+, Apache Portable Runtime (APR) 1.61+ and Boost 1.54+.

Here we use the Ubuntu 16.04 LTS system as an example to show you how to compile LivoxTech sdk's dependency libraries using apt or direct compiling the source code of apr and boost lib.

###### Ubuntu 16.04 LTS

The following package are required (feel free to cut and paste the apt-get command below):

```
sudo apt install cmake
```

###### Use Apt Install

The following commands can be used to download and install it:

```
sudo apt install \
	 libapr1-dev \
	 libboost-atomic-dev \
	 libboost-system-dev
```

###### Compile the Source Code

You can download the apr source code from http://apr.apache.org/download.cgi and boost source code from http://sourceforge.net/projects/boost/files/boost/1.54.0/. The following commands can be used to compile and install it.

```
tar zxf apr-1.6.5.tar.gz && \
cd apr-1.6.5 && \
./configure --prefix=/usr && \
make && \
make install
```

```
tar zxf boost_1_54_0.tar.gz && \
cd boost_1_54_0 && \
./bootstrap.sh --with-libraries=atomic,system && \
./b2 install --prefix=/usr
```

###### Compile the Project

In the LivoxTech SDK directory (e.g. the checkout root or the archive unpack root), run the following commands to compile the project:

```
mkdir build && cd build && \
cmake .. && \
make -j $(nproc)
```

###### Run the Project

Run the following commands to run the hub or lidar sample:

```
cd sample\hub && \
./hub_sample
```

```
cd sample\lidar && \
./lidar_sample
```

# Livox SDK Linux Sample

Two samples are provided in Sample/Lidar and Sample/Hub, which demonstrate how to configure Livox LiDAR units and receive the point cloud data when directly connecting Livox-SDK to LiDAR units or by using a Livox Hub, respectively. The sequence diagram is shown as below: 

![sample](doc/images/sample.png)

# Livox Ros Demo

Livox ROS demo is an application software running under ROS environment. It supports point cloud display using rviz. The Livox ROS demo includes two software packages, which are applicable when the host is connected to LiDAR sensors directly or when a Livox Hub is in use, respectively. This Livox ROS demo supports Ubuntu 14.04/Ubuntu 16.04, both x86 and ARM. It has been tested on Intel i7 and Nvidia TX2. 

## Livox ROS Demo User Guide

The Livox-SDK-ROS directory is organized in the form of ROS workspace, and is fully compatible with ROS workspace. Only a subfolder named src can be found under the Livox-SDK-ROS directory. Inside the src directory, there are two ROS software packages: display_lidar_points and display_hub_points.

1.	Download or clone the code from the Livox-SDK/Livox-SDK-ROS repository on GitHub. 
2.	Compile the ROS code package under the Livox-SDK-ROS directory by typing the following command in terminal:
  `catkin_make`
3.	Run the compiled ROS nodes:
  `rosrun display_lidar_points display_lidar_points_node`
  or
  `rosrun display_hub_points display_hub_points_node`
4.	Open a new terminal, and run roscore under the Livox-SDK-ROS directory:
  `roscore`
5.	Open another new terminal, and run rviz under the Livox-SDK-ROS directory:
  `rosrun rviz rviz`
6.	Set ROS RVIZ:
  1.	Create new visualization by display type, and select PointCloud;
  2.	Set the Fixed Frame to “sensor_frame” in Global Options and set Frame Rate to 20;
  3.	Select “/cloud” in Topic under the newly created PointCLoud.

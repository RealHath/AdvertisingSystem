#!/bin/bash
# 环境配置脚本

cd ~

# 强迫症更新
yum -y upgrade

# gcc版本升级
sudo yum -y install gcc
sudo yum -y install gcc-c++
sudo yum -y install centos-release-scl
sudo yum -y install devtoolset-10-gcc*
source /opt/rh/devtoolset-10/enable
mv /usr/bin/gcc /usr/bin/gcc-4.8.5
ln -s /opt/rh/devtoolset-10/root/bin/gcc /usr/bin/gcc
mv /usr/bin/g++ /usr/bin/g++-4.8.5
ln -s /opt/rh/devtoolset-10/root/bin/g++ /usr/bin/g++

# 编译工具
sudo yum -y install make
sudo yum -y install cmake

# 安装brpc https://github.com/apache/incubator-brpc/blob/master/docs/cn/getting_started.md#fedoracentos
cd ~

sudo yum -y install epel-release
sudo yum -y install git openssl-devel
sudo yum -y install gflags-devel protobuf-devel protobuf-compiler leveldb-devel
sudo yum -y install gperftools-devel
sudo yum -y install gtest-devel
sudo yum -y install glog-devel

git clone git://github.com/apache/incubator-brpc.git
cd incubator-brpc/
sh config_brpc.sh --headers="/usr/include" --libs="/usr/lib64 /usr/bin" --nodebugsymbols
make

# mysql
sudo yum -y install mariadb-server
sudo systemctl start mariadb
sudo systemctl enable mariadb
# mysql_secure_installation配置mysql  https://www.cnblogs.com/yhongji/p/9783065.html
sudo yum -y install mysql-devel

# json2pb

# 安装杂项
sudo yum -y install zip
sudo yum -y install libuuid-devel
sudo yum -y install libcurl-devel
yum -y install devtoolset-11
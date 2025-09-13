#!/bin/bash

#  How to Use :
#   1. Install Git  (https://git-scm.com/downloads)
#   2. Run "Your Path\submodule.sh"

# USE_PLATFORM  [ x86_64 | aarch64 | ... ]  You can manually set it up.
export USE_PLATFORM=$(uname -m)
# CLEAR_SUBMODULE [no | yes]
export CLEAR_SUBMODULE=no

if [ ! -d "./submodule" ]; then  
    mkdir "./submodule"
fi

if [ ! -d "./submodule/linux" ]; then  
    mkdir "./submodule/linux"
fi

if [ ! -d "./third_party" ]; then  
    mkdir "./third_party"
fi

# install boost
if [ "$CLEAR_SUBMODULE" == "yes" ]; then
    rm -rf ./submodule/linux/boost
    rm -rf ./third_party/boost/linux/$USE_PLATFORM
    rm -rf ./.git/modules/submodule/linux/boost
fi
if [ ! -d "./third_party/boost/linux/$USE_PLATFORM/lib/libboost_system.so" ]; then  
	git submodule add -f http://github.com/boostorg/boost.git ./submodule/linux/boost
	if [ $? -ne 0 ]; then
		exit 1
	fi
	cd submodule/linux/boost
	git checkout boost-1.78.0
	if [ $? -ne 0 ]; then
		exit 1
	fi
	git submodule update --init
	if [ $? -ne 0 ]; then
		exit 1
	fi
	./bootstrap.sh --prefix=../../../third_party/boost/linux/$USE_PLATFORM
	if [ $? -ne 0 ]; then
		exit 1
	fi
	./b2 -j$(nproc)
	if [ $? -ne 0 ]; then
		exit 1
	fi
	./b2 install
	if [ $? -ne 0 ]; then
		exit 1
	fi
	cd ../../..
fi

# install protobuf 
if [ "$CLEAR_SUBMODULE" == "yes" ]; then
    rm -rf ./submodule/linux/protobuf
    rm -rf ./third_party/protobuf/linux/$USE_PLATFORM
    rm -rf ./.git/modules/submodule/linux/protobuf
fi

git submodule add -f http://github.com/protocolbuffers/protobuf.git ./submodule/linux/protobuf
if [ $? -ne 0 ]; then
	exit 1
fi
cd submodule/linux/protobuf 
git checkout v3.11.4
if [ $? -ne 0 ]; then
	exit 1
fi
git submodule update --init
if [ $? -ne 0 ]; then
	exit 1
fi
cd cmake 
if [ ! -d "./cbuild" ]; then 
	mkdir cbuild
fi
cd cbuild
# build Debug
if [ ! -d "./Debug" ]; then 
	mkdir Debug
	cd Debug
else
    cd Debug
	rm -rf *
fi
cmake -G "Unix Makefiles" -Dprotobuf_BUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=../../../../../../third_party/protobuf/linux/$USE_PLATFORM ../..
if [ $? -ne 0 ]; then
	exit 1
fi
make -j$(nproc)
if [ $? -ne 0 ]; then
	exit 1
fi
make install
if [ $? -ne 0 ]; then
	exit 1
fi
cd ..
# build Release
if [ ! -d "./Release" ]; then 
    mkdir Release
    cd Release
else
    cd Release
	rm -rf *
fi
cmake -G "Unix Makefiles" -Dprotobuf_BUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../../../../../../third_party/protobuf/linux/$USE_PLATFORM ../..
if [ $? -ne 0 ]; then
	exit 1
fi
make -j$(nproc)
if [ $? -ne 0 ]; then
	exit 1
fi
make install
if [ $? -ne 0 ]; then
	exit 1
fi
cd ../../../../../..


#zeromq
if [ "$CLEAR_SUBMODULE" == "yes" ]; then
    rm -rf ./submodule/linux/zeromq 
    rm -rf ./third_party/zeromq/linux/$USE_PLATFORM
    rm -rf ./.git/modules/submodule/linux/zeromq
fi

git submodule add -f https://github.com/zeromq/zeromq4-x.git ./submodule/linux/zeromq
if [ $? -ne 0 ]; then
	exit 1
fi
cd submodule/linux/zeromq 
git checkout v4.0.10
if [ $? -ne 0 ]; then
	exit 1
fi
git submodule update --init
if [ $? -ne 0 ]; then
	exit 1
fi
if ! (command -v libtoolize > /dev/null 2>&1 || command -v libtool > /dev/null 2>&1 || command -v glibtoolize > /dev/null 2>&1 || command -v glibtool > /dev/null 2>&1); then
	if command -v apt-get > /dev/null 2>&1; then
		sudo apt-get install -y libtool
	elif command -v yum > /dev/null 2>&1; then
		sudo yum install -y libtool
	elif command -v dnf > /dev/null 2>&1; then
		sudo dnf install -y libtool
	elif command -v zypper > /dev/null 2>&1; then
		sudo zypper install -y libtool
	elif command -v brew > /dev/null 2>&1; then
		brew install libtool
	else
		echo "Error: Unsupported package manager, please manually install libtool."
		exit 1
	fi
fi
./autogen.sh
./configure --prefix=$(pwd)/../../../third_party/zeromq/linux/$USE_PLATFORM
if [ $? -ne 0 ]; then
	exit 1
fi
make -j$(nproc)
if [ $? -ne 0 ]; then
	exit 1
fi
make install
if [ $? -ne 0 ]; then
	exit 1
fi
cd ../../..

# cppzmq
if [ "$CLEAR_SUBMODULE" == "yes" ]; then
    rm -rf ./submodule/linux/cppzmq
    rm -rf ./third_party/cppzmq/include
    rm -rf ./.git/modules/submodule/linux/cppzmq
fi

git submodule add -f https://github.com/zeromq/cppzmq.git ./submodule/linux/cppzmq
if [ $? -ne 0 ]; then
	exit 1
fi
cd submodule/linux/cppzmq
git checkout v4.2.2
if [ $? -ne 0 ]; then
	exit 1
fi
git submodule update --init
if [ $? -ne 0 ]; then
	exit 1
fi
mkdir ../../../third_party/cppzmq
mkdir ../../../third_party/cppzmq/include
cp -f zmq.hpp ../../../third_party/cppzmq/include/zmq.hpp
cd ../../..

# mysql 
if [ "$CLEAR_SUBMODULE" == "yes" ]; then
    rm -rf ./submodule/linux/mysql
    rm -rf ./third_party/mysql/linux/$USE_PLATFORM
fi

if ! command -v libncurses5 > /dev/null 2>&1; then
	if command -v apt-get > /dev/null; then
		sudo apt-get install -y libncurses5-dev
	elif command -v yum > /dev/null 2>&1; then
		sudo yum install -y ncurses-devel
	elif command -v dnf > /dev/null 2>&1; then
		sudo dnf install -y ncurses-devel
	elif command -v zypper > /dev/null 2>&1; then
		sudo zypper install -y ncurses-devel
	elif command -v brew > /dev/null 2>&1; then
		brew install ncurses
	else
		echo "Error: Unsupported package manager, please manually install libncurses5-dev."
		exit 1
	fi
fi
if ! command -v pkg-config > /dev/null 2>&1; then
	if command -v apt-get > /dev/null; then
		sudo apt-get install -y pkg-config
	elif command -v yum > /dev/null 2>&1; then
		sudo yum install -y pkgconfig
	elif command -v dnf > /dev/null 2>&1; then
		sudo dnf install -y pkgconfig
	elif command -v zypper > /dev/null 2>&1; then
		sudo zypper install -y pkg-config
	elif command -v brew > /dev/null 2>&1; then
		brew install pkg-config
	else
		echo "Error: Unsupported package manager, please manually install pkg-config."
		exit 1
	fi
fi
if ! pkg-config --exists openssl && [ ! -f /usr/include/openssl/ssl.h ] && [ ! -f /usr/local/include/openssl/ssl.h ]; then
	if command -v apt-get > /dev/null 2>&1; then
		sudo apt-get install -y libssl-dev
	elif command -v yum > /dev/null 2>&1; then
		sudo yum install -y openssl-devel
	elif command -v dnf > /dev/null 2>&1; then
		sudo dnf install -y openssl-devel
	elif command -v zypper > /dev/null 2>&1; then
		sudo zypper install -y libopenssl-devel
	elif command -v brew > /dev/null 2>&1; then
		brew install openssl
	else
		echo "Error: Unsupported package manager, please manually install openssl."
		exit 1
	fi
fi
if ! pkg-config --exists libtirpc; then
	if command -v apt-get > /dev/null; then
		sudo apt-get install -y libtirpc-dev
	elif command -v yum > /dev/null 2>&1; then
		sudo yum install -y libtirpc-devel
	elif command -v dnf > /dev/null 2>&1; then
		sudo dnf install -y libtirpc-devel
	elif command -v zypper > /dev/null 2>&1; then
		sudo zypper install -y glibc-devel
	elif command -v brew > /dev/null 2>&1; then
		brew install libtirpc
	else
		echo "Error: Unsupported package manager, please manually install libtirpc."
		exit 1
	fi
fi

cd ./submodule/linux
if [ ! -d "./mysql" ]; then  
	mkdir mysql
fi
cd mysql
if [ ! -f "./mysql-8.0.43.tar.gz" ]; then
    wget --no-check-certificate https://cdn.mysql.com//Downloads/MySQL-8.0/mysql-8.0.43.tar.gz
fi
if [ ! -d "./mysql-8.0.43" ]; then
    tar -xzf mysql-8.0.43.tar.gz
fi
if [ "$CLEAR_SUBMODULE" == "yes" ]; then
	rm -f mysql-8.0.43.tar.gz
fi
cd mysql-8.0.43
cp -f ../../../boost.cmake cmake/boost.cmake
cp -f ../../../buffer.cc sql/gis/buffer.cc
if [ ! -d "./cbuild" ]; then 
    mkdir cbuild 
    cd cbuild
else
    cd cbuild
	rm -rf *
fi
cmake -G "Unix Makefiles" -DWITHOUT_SERVER=1 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../../../../../third_party/mysql/linux/$USE_PLATFORM -DBOOST_INCLUDE_DIR=../../../../../third_party/boost/linux/$USE_PLATFORM/include ..
if [ $? -ne 0 ]; then
	exit 1
fi
make -j$(nproc)
if [ $? -ne 0 ]; then
	exit 1
fi
make install
if [ $? -ne 0 ]; then
	exit 1
fi
cd ../../../../..


# curl
if [ "$CLEAR_SUBMODULE" == "yes" ]; then
    rm -rf ./submodule/linux/curl
    rm -rf ./third_party/curl/linux/$USE_PLATFORM
    rm -rf ./.git/modules/submodule/linux/curl
fi

git submodule add -f https://github.com/curl/curl.git ./submodule/linux/curl
if [ $? -ne 0 ]; then
	exit 1
fi
cd submodule/linux/curl
git checkout curl-7_77_0
if [ $? -ne 0 ]; then
	exit 1
fi
git submodule update --init
if [ $? -ne 0 ]; then
	exit 1
fi
if [ ! -d "./cbuild" ]; then 
	mkdir cbuild
fi
cd cbuild
# build Debug
if [ ! -d "./Debug" ]; then 
    mkdir Debug
    cd Debug
else
    cd Debug
	rm -rf *
fi
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=../../../../../third_party/curl/linux/$USE_PLATFORM ../..
if [ $? -ne 0 ]; then
	exit 1
fi
make -j$(nproc)
if [ $? -ne 0 ]; then
	exit 1
fi
make install
if [ $? -ne 0 ]; then
	exit 1
fi
cd ..
# build Release
if [ ! -d "./Release" ]; then 
    mkdir Release
    cd Release
else
    cd Release
	rm -rf *
fi
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../../../../../third_party/curl/linux/$USE_PLATFORM ../..
if [ $? -ne 0 ]; then
	exit 1
fi
make -j$(nproc)
if [ $? -ne 0 ]; then
	exit 1
fi
make install
if [ $? -ne 0 ]; then
	exit 1
fi
cd ../../../../..




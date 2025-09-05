#!/bin/bash

##############################
### note: If write by Notepad++, then run  "sed -i "s/\r//" RebuildCmProject.sh" first.

##################################################
## build Share Lib
# build Debug
cd shared
if [ ! -d "cbuild" ];then
	mkdir cbuild
	cd cbuild
else
	cd cbuild
	rm -rf *;
fi
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
if [ $? -ne 0 ]; then
	echo "Build shared debug fail !"
	exit 1
fi
# build Release
rm -rf *;
cmake ..
make -j$(nproc)
if [ $? -ne 0 ]; then
	echo "Build shared release fail !"
	exit 1
fi
cd ../../

##################################################
## build Game 
# build Debug 
cd agent_server
if [ ! -d "cbuild" ];then
	mkdir cbuild
	cd cbuild
else
	cd cbuild
	rm -rf *;
fi
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
if [ $? -ne 0 ]; then
	echo "Build agent_server debug fail !"
	exit 1
fi
# build Release
rm -rf *;
cmake ..
make -j$(nproc)
if [ $? -ne 0 ]; then
	echo "Build agent_server release fail !"
	exit 1
fi
cd ../../

######################
# build Debug
cd cache_server
if [ ! -d "cbuild" ];then
	mkdir cbuild
	cd cbuild
else
	cd cbuild
	rm -rf *;
fi
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
if [ $? -ne 0 ]; then
	echo "Build cache_server debug fail !"
	exit 1
fi
# build Release
rm -rf *;
cmake ..
make -j$(nproc)
if [ $? -ne 0 ]; then
	echo "Build cache_server release fail !"
	exit 1
fi
cd ../../

######################
# build Debug
cd game_cache_server
if [ ! -d "cbuild" ];then
	mkdir cbuild
	cd cbuild
else
	cd cbuild
	rm -rf *;
fi
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
if [ $? -ne 0 ]; then
	echo "Build game_cache_server debug fail !"
	exit 1
fi
# build Release
rm -rf *;
cmake ..
make -j$(nproc)
if [ $? -ne 0 ]; then
	echo "Build game_cache_server release fail !"
	exit 1
fi
cd ../../

######################
# build Debug
cd game_server
if [ ! -d "cbuild" ];then
	mkdir cbuild
	cd cbuild
else
	cd cbuild
	rm -rf *;
fi
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
if [ $? -ne 0 ]; then
	echo "Build game_server debug fail !"
	exit 1
fi
# build Release
rm -rf *;
cmake ..
make -j$(nproc)
if [ $? -ne 0 ]; then
	echo "Build game_server release fail !"
	exit 1
fi
cd ../../

######################
# build Debug
cd login_server
if [ ! -d "cbuild" ];then
	mkdir cbuild
	cd cbuild
else
	cd cbuild
	rm -rf *;
fi
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
if [ $? -ne 0 ]; then
	echo "Build login_server debug fail !"
	exit 1
fi
# build Release
rm -rf *;
cmake ..
make -j$(nproc)
if [ $? -ne 0 ]; then
	echo "Build login_server release fail !"
	exit 1
fi
cd ../../

######################
# build Debug
cd master_server
if [ ! -d "cbuild" ];then
	mkdir cbuild
	cd cbuild
else
	cd cbuild
	rm -rf *;
fi
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
if [ $? -ne 0 ]; then
	echo "Build master_server debug fail !"
	exit 1
fi
# build Release
rm -rf *;
cmake ..
make -j$(nproc)
if [ $? -ne 0 ]; then
	echo "Build master_server release fail !"
	exit 1
fi
cd ../../

######################
# build Debug
cd rank_server
if [ ! -d "cbuild" ];then
	mkdir cbuild
	cd cbuild
else
	cd cbuild
	rm -rf *;
fi
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
if [ $? -ne 0 ]; then
	echo "Build rank_server debug fail !"
	exit 1
fi
# build Release
rm -rf *;
cmake ..
make -j$(nproc)
if [ $? -ne 0 ]; then
	echo "Build rank_server release fail !"
	exit 1
fi
cd ../../

######################
# build Debug
cd servant_server
if [ ! -d "cbuild" ];then
	mkdir cbuild
	cd cbuild
else
	cd cbuild
	rm -rf *;
fi
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
if [ $? -ne 0 ]; then
	echo "Build servant_server debug fail !"
	exit 1
fi
# build Release
rm -rf *;
cmake ..
make -j$(nproc)
if [ $? -ne 0 ]; then
	echo "Build servant_server release fail !"
	exit 1
fi
cd ../../

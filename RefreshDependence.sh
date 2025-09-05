#!/bin/bash

##############################
### Note: If write by Notepad++, then run  "sed -i "s/\r//" RefreshDependence.sh" first.

# USE_PLATFORM  [ x86_64 | aarch64 | ... ]  You can manually set it up.
export USE_PLATFORM=$(uname -m)

funRefreshDebugLib() {
	cp -vf third_party/boost/linux/$USE_PLATFORM/libboost_chrono.so $1
	cp -vf third_party/boost/linux/$USE_PLATFORM/libboost_date_time.so $1
	cp -vf third_party/boost/linux/$USE_PLATFORM/libboost_system.so $1
	cp -vf third_party/boost/linux/$USE_PLATFORM/libboost_thread.so $1
	cp -vf third_party/protobuf/linux/$USE_PLATFORM/libprotobuf.so $1
	cp -vf third_party/zeromq/linux/$USE_PLATFORM/libzmq.so $1
	cp -vf shared/bin/linux/$USE_PLATFORM/Debug/libShared.so $1
}

funRefreshReleaseLib() {
	cp -vf third_party/boost/linux/$USE_PLATFORM/libboost_chrono.so $1
	cp -vf third_party/boost/linux/$USE_PLATFORM/libboost_date_time.so $1
	cp -vf third_party/boost/linux/$USE_PLATFORM/libboost_system.so $1
	cp -vf third_party/boost/linux/$USE_PLATFORM/libboost_thread.so $1
	cp -vf third_party/protobuf/linux/$USE_PLATFORM/libprotobuf.so $1
	cp -vf third_party/zeromq/linux/$USE_PLATFORM/libzmq.so $1
	cp -vf shared/bin/linux/$USE_PLATFORM/Release/libShared.so $1
}

######
funRefreshDebugLib servant_server/bin/linux/$USE_PLATFORM/Debug/
funRefreshReleaseLib servant_server/bin/linux/$USE_PLATFORM/Release/

funRefreshDebugLib master_server/bin/linux/$USE_PLATFORM/Debug/
funRefreshReleaseLib master_server/bin/linux/$USE_PLATFORM/Release/

funRefreshDebugLib login_server/bin/linux/$USE_PLATFORM/Debug/
funRefreshReleaseLib login_server/bin/linux/$USE_PLATFORM/Release/

funRefreshDebugLib game_server/bin/linux/$USE_PLATFORM/Debug/
funRefreshReleaseLib game_server/bin/linux/$USE_PLATFORM/Release/

funRefreshDebugLib rank_server/bin/linux/$USE_PLATFORM/Debug/
funRefreshReleaseLib rank_server/bin/linux/$USE_PLATFORM/Release/

funRefreshDebugLib game_cache_server/bin/linux/$USE_PLATFORM/Debug/
funRefreshReleaseLib game_cache_server/bin/linux/$USE_PLATFORM/Release/

funRefreshDebugLib cache_server/bin/linux/$USE_PLATFORM/Debug/
funRefreshReleaseLib cache_server/bin/linux/$USE_PLATFORM/Release/

funRefreshDebugLib agent_server/bin/linux/$USE_PLATFORM/Debug/
funRefreshReleaseLib agent_server/bin/linux/$USE_PLATFORM/Release/

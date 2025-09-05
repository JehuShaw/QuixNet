#!/bin/bash

##############################
### Note: If write by Notepad++, then run  "sed -i "s/\r//" ClearCmProject.sh" first.

# USE_PLATFORM  [ x86_64 | aarch64 | ... ]  You can manually set it up.
export USE_PLATFORM=$(uname -m)

rm -rf ./cache_server/build/*
rm -rf ./cache_server/cbuild/*
rm -rf ./cache_server/bin/linux/$USE_PLATFORM/Debug/log
rm -rf ./cache_server/bin/linux/$USE_PLATFORM/Release/log
rm -f ./cache_server/bin/linux/$USE_PLATFORM/Debug/nohup.out
rm -f ./cache_server/bin/linux/$USE_PLATFORM/Release/nohup.out
rm -rf ./cache_server/bin/win/x64/Debug/log
rm -rf ./cache_server/bin/win/x64/Release/log
rm -rf ./cache_server/bin/win/x32/Debug/log
rm -rf ./cache_server/bin/win/x32/Release/log
rm -rf ./cache_server/vsproject/.vs


rm -rf ./game_server/build/*
rm -rf ./game_server/cbuild/*
rm -rf ./game_server/bin/linux/$USE_PLATFORM/Debug/log
rm -rf ./game_server/bin/linux/$USE_PLATFORM/Release/log
rm -f ./game_server/bin/linux/$USE_PLATFORM/Debug/nohup.out
rm -f ./game_server/bin/linux/$USE_PLATFORM/Release/nohup.out
rm -rf ./game_server/bin/win/x64/Debug/log
rm -rf ./game_server/bin/win/x64/Release/log
rm -rf ./game_server/bin/win/x32/Debug/log
rm -rf ./game_server/bin/win/x32/Release/log
rm -rf ./game_server/vsproject/.vs


rm -rf ./login_server/build/*
rm -rf ./login_server/cbuild/*
rm -rf ./login_server/bin/linux/$USE_PLATFORM/Debug/log
rm -rf ./login_server/bin/linux/$USE_PLATFORM/Release/log
rm -f ./login_server/bin/linux/$USE_PLATFORM/Debug/nohup.out
rm -f ./login_server/bin/linux/$USE_PLATFORM/Release/nohup.out
rm -rf ./login_server/bin/win/x64/Debug/log
rm -rf ./login_server/bin/win/x64/Release/log
rm -rf ./login_server/bin/win/x32/Debug/log
rm -rf ./login_server/bin/win/x32/Release/log
rm -rf ./login_server/vsproject/.vs


rm -rf ./master_server/build/*
rm -rf ./master_server/cbuild/*
rm -rf ./master_server/bin/linux/$USE_PLATFORM/Debug/log
rm -rf ./master_server/bin/linux/$USE_PLATFORM/Release/log
rm -f ./master_server/bin/linux/$USE_PLATFORM/Debug/nohup.out
rm -f ./master_server/bin/linux/$USE_PLATFORM/Release/nohup.out
rm -rf ./master_server/bin/win/x64/Debug/log
rm -rf ./master_server/bin/win/x64/Release/log
rm -rf ./master_server/bin/win/x32/Debug/log
rm -rf ./master_server/bin/win/x32/Release/log
rm -rf ./master_server/vsproject/.vs


rm -rf ./agent_server/build/*
rm -rf ./agent_server/cbuild/*
rm -rf ./agent_server/bin/linux/$USE_PLATFORM/Debug/log
rm -rf ./agent_server/bin/linux/$USE_PLATFORM/Release/log
rm -f ./agent_server/bin/linux/$USE_PLATFORM/Debug/nohup.out
rm -f ./agent_server/bin/linux/$USE_PLATFORM/Release/nohup.out
rm -rf ./agent_server/bin/win/x64/Debug/log
rm -rf ./agent_server/bin/win/x64/Release/log
rm -rf ./agent_server/bin/win/x32/Debug/log
rm -rf ./agent_server/bin/win/x32/Release/log
rm -rf ./agent_server/vsproject/.vs


rm -rf ./servant_server/build/*
rm -rf ./servant_server/cbuild/*
rm -rf ./servant_server/bin/linux/$USE_PLATFORM/Debug/log
rm -rf ./servant_server/bin/linux/$USE_PLATFORM/Release/log
rm -f ./servant_server/bin/linux/$USE_PLATFORM/Debug/nohup.out
rm -f ./servant_server/bin/linux/$USE_PLATFORM/Release/nohup.out
rm -rf ./servant_server/bin/win/x64/Debug/log
rm -rf ./servant_server/bin/win/x64/Release/log
rm -rf ./servant_server/bin/win/x32/Debug/log
rm -rf ./servant_server/bin/win/x32/Release/log
rm -rf ./servant_server/vsproject/.vs


rm -rf ./rank_server/build/*
rm -rf ./rank_server/cbuild/*
rm -rf ./rank_server/bin/linux/$USE_PLATFORM/Debug/log
rm -rf ./rank_server/bin/linux/$USE_PLATFORM/Release/log
rm -f ./rank_server/bin/linux/$USE_PLATFORM/Debug/nohup.out
rm -f ./rank_server/bin/linux/$USE_PLATFORM/Release/nohup.out
rm -rf ./rank_server/bin/win/x64/Debug/log
rm -rf ./rank_server/bin/win/x64/Release/log
rm -rf ./rank_server/bin/win/x32/Debug/log
rm -rf ./rank_server/bin/win/x32/Release/log
rm -rf ./rank_server/vsproject/.vs


rm -rf ./shared/build/*
rm -rf ./shared/cbuild/*
rm -rf ./shared/vsproject/.vs


rm -rf ./vsproject_shortcut/.vs




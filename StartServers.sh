#!/bin/bash

##############################
### Note: If write by Notepad++, then run  "sed -i "s/\r//" StartServers.sh" first.

BuildType=$1
if [ "$BuildType" == "" ] || [ "$BuildType" == "Release" ]; then
	BuildType="Release"
elif [ "$BuildType" == "Debug" ]; then
	BuildType="Debug"
else
	echo "Build type failed"; exit 1;
fi

funOpenServer() 
{
	echo "Start $1/bin/linux32/$BuildType/$2"
	cd $1/bin/linux32/$BuildType/
	nohup ./$2 &
	sleep 1
	count=0
	while [ $( pgrep -f $2 | wc -l ) -eq 0 ]
	do
		count=$((${count} + 1))
		if [ $count -gt 60 ]; then
			echo "Force break waiting $pid"
			break
		fi
		echo "wait $pid $count second."
		sleep 1
	done
	cd ../../../../
	echo ""
}

echo ""

funOpenServer master_server MasterServer
funOpenServer servant_server ServantServer
funOpenServer cache_server CacheServer
funOpenServer agent_server AgentServer
funOpenServer game_server GameServer
funOpenServer rank_server RankServer
funOpenServer game_cache_server GameCacheServer
funOpenServer login_server LoginServer

ps aux | grep Server

#!/bin/bash

##############################
### Note: If write by Notepad++, then run  "sed -i "s/\r//" ExitServers.sh" first.

funExitServer() {
	keyword=$1
	pids=$(pgrep -f $keyword)
	for pid in $pids
	do
		echo "Exit $keyword $pid"
		kill $pid
		sleep 1
		count=0
		while [ $( pgrep -f $keyword | wc -l ) -ne 0 ]
		do
			count=$((${count} + 1))
			if [ $count -gt 60 ]; then
				kill -9 $pid
				echo "force kill $pid"
				break
			fi
			echo "wait $pid $count second."
			sleep 1
		done
		echo ""
	done
}

echo ""

funExitServer LoginServer
funExitServer GameCacheServer
funExitServer RankServer
funExitServer GameServer
funExitServer AgentServer
funExitServer CacheServer
funExitServer ServantServer
funExitServer MasterServer

ps aux | grep Server
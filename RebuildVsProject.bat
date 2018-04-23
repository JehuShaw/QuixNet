@ECHO off

SET devenvpath=
SET selected=

rem 选择开发环境
SET /p selected=选择开发环境[1=vs2008;2=vs2010;3=vs2012;4=vs2013;5=2005]:

echo %VS100COMNTOOLS%
if %selected%==1 (
	SET devenvpath="%VS90COMNTOOLS%..\IDE\devenv.exe"
) else if %selected%==2 (
	SET devenvpath="%VS100COMNTOOLS%..\IDE\devenv.exe"
) else if %selected%==3 (
	SET devenvpath="%VS110COMNTOOLS%..\IDE\devenv.exe"
) else if %selected%==4 (
	SET devenvpath="%VS120COMNTOOLS%..\IDE\devenv.exe"
) else if %selected%==5 (
	SET devenvpath="%VS80COMNTOOLS%..\IDE\devenv.exe"
) else (
	goto error
)



@ECHO on

%devenvpath% cache_server\vsproject\CacheServer.sln /Rebuild Debug /Out log.txt
%devenvpath% cache_server\vsproject\CacheServer.sln /Rebuild Release /Out log.txt

%devenvpath% game_client\vsproject\GameClient.sln /Rebuild Debug /Out log.txt
%devenvpath% game_client\vsproject\GameClient.sln /Rebuild Release /Out log.txt

%devenvpath% game_server\vsproject\GameServer.sln /Rebuild Debug /Out log.txt
%devenvpath% game_server\vsproject\GameServer.sln /Rebuild Release /Out log.txt

%devenvpath% login_server\vsproject\LoginServer.sln /Rebuild Debug /Out log.txt
%devenvpath% login_server\vsproject\LoginServer.sln /Rebuild Release /Out log.txt

%devenvpath% master_server\vsproject\MasterServer.sln /Rebuild Debug /Out log.txt
%devenvpath% master_server\vsproject\MasterServer.sln /Rebuild Release /Out log.txt

%devenvpath% agent_server\vsproject\AgentServer.sln /Rebuild Debug /Out log.txt
%devenvpath% agent_server\vsproject\AgentServer.sln /Rebuild Release /Out log.txt

%devenvpath% rank_server\vsproject\RankServer.sln /Rebuild Debug /Out log.txt
%devenvpath% rank_server\vsproject\RankServer.sln /Rebuild Release /Out log.txt

%devenvpath% servant_server\vsproject\ServantServer.sln /Rebuild Debug /Out log.txt
%devenvpath% servant_server\vsproject\ServantServer.sln /Rebuild Release /Out log.txt


@goto final

:error
	ECHO 选择错误，请重新选择

:final


@ECHO off

SET devenvpath="%VSAPPIDDIR%\devenv.exe"



@ECHO on

%devenvpath% cache_server\vsproject\cache_server.sln /Rebuild Debug /Out log.txt
%devenvpath% cache_server\vsproject\cache_server.sln /Rebuild Release /Out log.txt

%devenvpath% game_server\vsproject\game_server.sln /Rebuild Debug /Out log.txt
%devenvpath% game_server\vsproject\game_server.sln /Rebuild Release /Out log.txt

%devenvpath% login_server\vsproject\login_server.sln /Rebuild Debug /Out log.txt
%devenvpath% login_server\vsproject\login_server.sln /Rebuild Release /Out log.txt

%devenvpath% master_server\vsproject\master_server.sln /Rebuild Debug /Out log.txt
%devenvpath% master_server\vsproject\master_server.sln /Rebuild Release /Out log.txt

%devenvpath% agent_server\vsproject\agent_server.sln /Rebuild Debug /Out log.txt
%devenvpath% agent_server\vsproject\agent_server.sln /Rebuild Release /Out log.txt

%devenvpath% servant_server\vsproject\servant_server.sln /Rebuild Debug /Out log.txt
%devenvpath% servant_server\vsproject\servant_server.sln /Rebuild Release /Out log.txt




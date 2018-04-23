rd /s /q agent_server\build
rd /s /q agent_server\bin\win32\debug\log
rd /s /q agent_server\bin\win32\release\log
rd /s /q agent_server\vsproject\ipch
del /f /a /q agent_server\vsproject\AgentServer.sdf
del /f /a /q agent_server\vsproject\AgentServer.suo

rd /s /q cache_server\build
rd /s /q cache_server\bin\win32\debug\log
rd /s /q cache_server\bin\win32\release\log
rd /s /q cache_server\vsproject\ipch
del /f /a /q cache_server\vsproject\CacheServer.sdf
del /f /a /q cache_server\vsproject\CacheServer.suo

rd /s /q game_client\build
rd /s /q game_client\bin\win32\debug\log
rd /s /q game_client\bin\win32\release\log
rd /s /q game_client\vsproject\ipch
del /f /a /q game_client\vsproject\GameClient.sdf
del /f /a /q game_client\vsproject\GameClient.suo

rd /s /q master_server\build
rd /s /q master_server\bin\win32\debug\log
rd /s /q master_server\bin\win32\release\log
rd /s /q master_server\vsproject\ipch
del /f /a /q master_server\vsproject\MasterServer.sdf
del /f /a /q master_server\vsproject\MasterServer.suo

rd /s /q servant_server\build
rd /s /q servant_server\bin\win32\debug\log
rd /s /q servant_server\bin\win32\release\log
rd /s /q servant_server\vsproject\ipch
del /f /a /q servant_server\vsproject\ServantServer.sdf
del /f /a /q servant_server\vsproject\ServantServer.suo

rd /s /q game_server\build
rd /s /q game_server\bin\win32\debug\log
rd /s /q game_server\bin\win32\release\log
rd /s /q game_server\vsproject\ipch
del /f /a /q game_server\vsproject\GameServer.sdf
del /f /a /q game_server\vsproject\GameServer.suo

rd /s /q login_server\build
rd /s /q login_server\bin\win32\debug\log
rd /s /q login_server\bin\win32\release\log
rd /s /q login_server\vsproject\ipch
del /f /a /q login_server\vsproject\LoginServer.sdf
del /f /a /q login_server\vsproject\LoginServer.suo

rd /s /q rank_server\build
rd /s /q rank_server\bin\win32\debug\log
rd /s /q rank_server\bin\win32\release\log
rd /s /q rank_server\vsproject\ipch
del /f /a /q rank_server\vsproject\RankServer.sdf
del /f /a /q rank_server\vsproject\RankServer.suo

rd /s /q shared\vsproject\Shared\Debug
rd /s /q shared\vsproject\Shared\Release
rd /s /q shared\vsproject\ipch
del /f /a /q shared\vsproject\Shared.sdf
del /f /a /q shared\vsproject\Shared.suo

rd /s /q vsproject_shortcut\ipch
del /f /a /q vsproject_shortcut\AllProject.sdf
del /f /a /q vsproject_shortcut\AllProject.suo


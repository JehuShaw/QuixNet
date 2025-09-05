rd /s /q agent_server\build
if exist agent_server\bin\win\x64 (
    rd /s /q agent_server\bin\win\x64\Debug\log
    rd /s /q agent_server\bin\win\x64\Release\log
)
if exist agent_server\bin\win\x32 (
    rd /s /q agent_server\bin\win\x32\Debug\log
    rd /s /q agent_server\bin\win\x32\Release\log
)
rd /s /q agent_server\vsproject\.vs


rd /s /q cache_server\build
if exist cache_server\bin\win\x64 (
    rd /s /q cache_server\bin\win\x64\Debug\log
    rd /s /q cache_server\bin\win\x64\Release\log
)
if exist cache_server\bin\win\x32 (
    rd /s /q cache_server\bin\win\x32\Debug\log
    rd /s /q cache_server\bin\win\x32\Release\log
)
rd /s /q cache_server\vsproject\.vs


rd /s /q master_server\build
if exist master_server\bin\win\x64 (
    rd /s /q master_server\bin\win\x64\Debug\log
    rd /s /q master_server\bin\win\x64\Release\log
)
if exist master_server\bin\win\x32 (
    rd /s /q master_server\bin\win\x32\Debug\log
    rd /s /q master_server\bin\win\x32\Release\log
)
rd /s /q master_server\vsproject\.vs


rd /s /q servant_server\build
if exist servant_server\bin\win\x64 (
    rd /s /q servant_server\bin\win\x64\Debug\log
    rd /s /q servant_server\bin\win\x64\Release\log
)
if exist servant_server\bin\win\x32 (
    rd /s /q servant_server\bin\win\x32\Debug\log
    rd /s /q servant_server\bin\win\x32\Release\log
)
rd /s /q servant_server\vsproject\.vs


rd /s /q game_server\build
if exist game_server\bin\win\x64 (
    rd /s /q game_server\bin\win\x64\Debug\log
    rd /s /q game_server\bin\win\x64\Release\log
)
if exist game_server\bin\win\x32 (
    rd /s /q game_server\bin\win\x32\Debug\log
    rd /s /q game_server\bin\win\x32\Release\log
)
rd /s /q game_server\vsproject\.vs


rd /s /q login_server\build
if exist login_server\bin\win\x64 (
    rd /s /q login_server\bin\win\x64\Debug\log
    rd /s /q login_server\bin\win\x64\Release\log
)
if exist login_server\bin\win\x32 (
    rd /s /q login_server\bin\win\x32\Debug\log
    rd /s /q login_server\bin\win\x32\Release\log
)
rd /s /q login_server\vsproject\.vs


rd /s /q shared\build
rd /s /q shared\vsproject\.vs


rd /s /q vsproject_shortcut\.vs



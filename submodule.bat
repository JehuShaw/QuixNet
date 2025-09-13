@rem  How to Use :
@rem   1. Install Git  (https://git-scm.com/downloads)
@rem   2. Install Visual Studio 2022 
@rem   3. Open Visual Studio Developer Command Prompt 
@rem   4. Run "Your Path\submodule.bat"

@ECHO OFF

@rem USE_PLATFORM  [x32 | x64]
set USE_PLATFORM=x64
@rem CLEAR_SUBMODULE [no | yes]
set CLEAR_SUBMODULE=no


rem @if not exist submodule md submodule
rem @if not exist submodule\win md submodule\win
rem @if not exist third_party md third_party

rem git config --global https.sslVerify "false"
rem git config --global core.autocrlf true

if %USE_PLATFORM% == x64 (
    call "%VCInstallDir%\Auxiliary\Build\vcvarsall.bat" x64
	if not exist "nasm-2.16.01-installer-x64.exe" (
		powershell -Command "Invoke-WebRequest -Uri 'https://www.nasm.us/pub/nasm/releasebuilds/2.16.01/win64/nasm-2.16.01-installer-x64.exe' -OutFile 'nasm-2.16.01-installer-x64.exe'"
		if %errorlevel% neq 0 (
			echo "Downloading nasm-2.16.01-installer-x64.exe failed, please try again."
			goto endlocal
		)
		start /wait %cd%\nasm-2.16.01-installer-x64.exe /S
	)
) else (
    call "%VCInstallDir%\Auxiliary\Build\vcvarsall.bat" x86
	if not exist ".\nasm-2.16.01-installer-x86.exe" (
		powershell -Command "Invoke-WebRequest -Uri 'https://www.nasm.us/pub/nasm/releasebuilds/2.16.01/win32/nasm-2.16.01-installer-x86.exe' -OutFile 'nasm-2.16.01-installer-x86.exe'"
		if %errorlevel% neq 0 (
			echo "Downloading nasm-2.16.01-installer-x86.exe failed, please try again."
			goto endlocal
		)
		start /wait %cd%\nasm-2.16.01-installer-x86.exe /S
	)
)
@rem Use by openssl
set PATH=%PATH%;%LOCALAPPDATA%\bin\NASM

@rem install boost
for /F "tokens=1,2 delims=." %%a in ("%VisualStudioVersion%") do (
    if "%%a" equ "17" (
	    set VCToolsVersionMajor=14
        set VCToolsVersionMinor=3
	) else if "%%a" equ "16" (
	    set VCToolsVersionMajor=14
        set VCToolsVersionMinor=2
	) else if "%%a" equ "15" (
	    set VCToolsVersionMajor=14
        set VCToolsVersionMinor=1
	) else if "%%a" equ "14" (
	    set VCToolsVersionMajor=14
        set VCToolsVersionMinor=0
	)
)
@rem Use by Mysql etc.
if not exist ninja-win.zip (
    powershell -Command "Invoke-WebRequest -Uri 'https://github.com/ninja-build/ninja/releases/latest/download/ninja-win.zip' -OutFile 'ninja-win.zip'"
    if %errorlevel% neq 0 (
	    echo "Downloading ninja-win.zip failed, please try again."
	    goto endlocal
    )
)
if not exist ninja.exe (
    powershell -Command "Expand-Archive %cd%\ninja-win.zip -Force -DestinationPath %cd%"
    if %errorlevel% neq 0 (
	    echo "Unzipping ninja-win.zip failed !"
	    goto endlocal
    )
)
set PATH=%PATH%;%cd%

if %CLEAR_SUBMODULE% == yes (
    if exist submodule\win\boost rd /s /q submodule\win\boost
    if exist third_party\boost\win\%USE_PLATFORM% rd /s /q third_party\boost\win\%USE_PLATFORM%
    if exist .git\modules\submodule\win\boost rd /s /q .git\modules\submodule\win\boost
)

if not exist "third_party\boost\win\%USE_PLATFORM%\lib\libboost_system-vc143-mt-%USE_PLATFORM%-1_78.lib" (
	git submodule add -f https://github.com/boostorg/boost.git submodule/win/boost
	if %errorlevel% neq 0 (
	    echo "Git add submodule boost failed !"
		goto endlocal
	)
	cd submodule\win\boost
	git checkout boost-1.78.0
	if %errorlevel% neq 0 (
	    echo "Git checkout boost-1.78.0 failed !"
		goto endlocal
	)
	git submodule update --init
	if %errorlevel% neq 0 (
	    echo "Git update submodule boost-1.78.0 failed !"
		goto endlocal
	)
	call bootstrap.bat
	if %errorlevel% neq 0 (
	    echo "Call bootstrap.bat failed !"
		goto endlocal
	)
	ping -n 1 -w 1000 127.0.0.1 > NUL
	if exist project-config.jam (
		@ECHO import option ; > project-config.jam
		@ECHO. >> project-config.jam
		@ECHO using msvc : %VCToolsVersionMajor%.%VCToolsVersionMinor% : "%VCToolsInstallDir%bin\Hostx%__DOTNET_PREFERRED_BITNESS%\%Platform%\cl.exe" ; >> project-config.jam
		@ECHO. >> project-config.jam
		@ECHO option.set keep-going : false ; >> project-config.jam
		@ECHO. >> project-config.jam
		if %USE_PLATFORM% == x64 (
			b2 address-model=64 --without-graph --without-graph_parallel --without-mpi --without-python
			if %errorlevel% neq 0 (
			    echo "Compiling 64-bit boost fails !"
				goto endlocal
			)
			b2 address-model=64 --prefix=..\..\..\third_party\boost\win\%USE_PLATFORM% install
			if %errorlevel% neq 0 (
			    echo "Installation of 64-bit boost failed !"
				goto endlocal
			)
		) else (
			b2 address-model=32 --without-graph --without-graph_parallel --without-mpi --without-python
			if %errorlevel% neq 0 (
			    echo "Compiling 32-bit boost fails !"
				goto endlocal
			)
			b2 address-model=32 --prefix=..\..\..\third_party\boost\win\%USE_PLATFORM% install
			if %errorlevel% neq 0 (
			    echo "Installation of 32-bit boost failed !"
				goto endlocal
			)
		)
	)
	cd ..\..\..
)

rem mysql use
if not exist third_party\boost\win\%USE_PLATFORM%\include\boost-1_78\boost\geometry (
    xcopy /i /e submodule\win\boost\libs\geometry\include\boost\geometry third_party\boost\win\%USE_PLATFORM%\include\boost-1_78\boost\geometry
)
if not exist third_party\boost\win\%USE_PLATFORM%\include\boost-1_78\boost\numeric\ublas (
    xcopy /i /e submodule\win\boost\libs\numeric\ublas\include\boost\numeric\ublas third_party\boost\win\%USE_PLATFORM%\include\boost-1_78\boost\numeric\ublas
)
if not exist third_party\boost\win\%USE_PLATFORM%\include\boost-1_78\boost\date_time.hpp (
    copy submodule\win\boost\libs\date_time\include\boost\date_time.hpp third_party\boost\win\%USE_PLATFORM%\include\boost-1_78\boost /y
)

@rem install perl; Use by openssl
where perl >nul 2>&1
if %errorlevel% neq 0 (
    cd submodule/win
	mkdir perl
	cd perl
	powershell -Command "& $([scriptblock]::Create((New-Object Net.WebClient).DownloadString('https://platform.activestate.com/dl/cli/_pdli01/install.ps1')))" -c'state activate --default ActiveState-Projects/ActiveState-Perl-5.36.0'
	if %errorlevel% neq 0 (
		echo "Failed to install perl, please try again !"
		goto endlocal
	)
	cd ..\..\..
)

@rem install openssl
if %CLEAR_SUBMODULE% == yes (
    if exist submodule\win\openssl rd /s /q submodule\win\openssl
    if exist third_party\openssl\win\%USE_PLATFORM% rd /s /q third_party\openssl\win\%USE_PLATFORM%
    if exist .git\modules\submodule\win\openssl rd /s /q .git\modules\submodule\win\openssl
)
if not exist "third_party\openssl\win\%USE_PLATFORM%\lib\libcrypto.lib" (
	git submodule add -f https://github.com/openssl/openssl.git submodule/win/openssl
	if %errorlevel% neq 0 (
		echo "Git add submodule openssl failed !"
		goto endlocal
	)
	set PROJECT_DIR=%cd%
	cd submodule/win/openssl
	git checkout OpenSSL_1_1_1u
	if %errorlevel% neq 0 (
		echo "Git checkout OpenSSL_1_1_1u failed !"
		goto endlocal
	)
	git submodule update --init
	if %errorlevel% neq 0 (
		echo "Git update submodule OpenSSL failed !"
		goto endlocal
	)

	if %USE_PLATFORM% == x64 (
		perl Configure VC-WIN64A --prefix=%PROJECT_DIR%\third_party\openssl\win\x64
	) else if %USE_PLATFORM% == x32 (
		perl Configure VC-WIN32 --prefix=%PROJECT_DIR%\third_party\openssl\win\x32
	) else (
		echo "Unknown USE_PLATFORM value !"
		goto endlocal
	)

	if %errorlevel% neq 0 (
		echo "Running perl Configure fails !"
		goto endlocal
	)
	nmake
	if %errorlevel% neq 0 (
		echo "Compiling OpenSSL failed !"
		goto endlocal
	)
	rem It must be installed as an administrator
	nmake install
	nmake clean
	cd ../../..
)

@rem install curl
if %CLEAR_SUBMODULE% == yes (
    if exist submodule\win\curl rd /s /q submodule\win\curl 
    if exist third_party\curl\win\%USE_PLATFORM% rd /s /q third_party\curl\win\%USE_PLATFORM%
    if exist .git\modules\submodule\win\curl rd /s /q .git\modules\submodule\win\curl
)
if not exist "third_party\curl\win\%USE_PLATFORM%\lib\libcurl_imp.lib" (
	git submodule add -f https://github.com/curl/curl.git submodule/win/curl
	if %errorlevel% neq 0 (
		echo "Git add submodule curl failed !"
		goto endlocal
	)
	cd submodule/win/curl
	git checkout curl-7_77_0
	if %errorlevel% neq 0 (
		echo "Git checkout curl-7_77_0 failed !"
		goto endlocal
	)
	git submodule update --init
	if %errorlevel% neq 0 (
		echo "Git update submodule Curl failed !"
		goto endlocal
	)
	if not exist cbuild (
		mkdir cbuild
	)
	cd cbuild
	@rem build Debug
	if not exist Debug (
		mkdir Debug
		cd Debug
	) else (
		cd Debug
		del /q *
	)	
	cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=..\..\..\..\..\third_party\curl\win\%USE_PLATFORM% -DCURL_USE_OPENSSL=ON -DOPENSSL_ROOT_DIR=%cd%\..\..\..\..\..\third_party\openssl\win\%USE_PLATFORM% -DOPENSSL_INCLUDE_DIR=%cd%\..\..\..\..\..\third_party\openssl\win\%USE_PLATFORM%\include -DOPENSSL_CRYPTO_LIBRARY=%cd%\..\..\..\..\..\third_party\openssl\win\%USE_PLATFORM%\lib\libcrypto.lib ..\..
	if %errorlevel% neq 0 (
		echo "CMake failed to generate the Debug version of Curl !"
		goto endlocal
	)
	ninja -j %NUMBER_OF_PROCESSORS%
	if %errorlevel% neq 0 (
		echo "Compiling the Debug version of ZeroMQ failed !"
		goto endlocal
	)
	ninja install
	if %errorlevel% neq 0 (
		echo "Failed to install the Debug version of Curl !"
		goto endlocal
	)
	cd ..
	@rem build Release
	if not exist Release (
		mkdir Release
		cd Release
	) else (
		cd Release
		del /q *
	)	
	cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=..\..\..\..\..\third_party\curl\win\%USE_PLATFORM% -DCURL_USE_OPENSSL=ON -DOPENSSL_ROOT_DIR=%cd%\..\..\..\..\..\third_party\openssl\win\%USE_PLATFORM% -DOPENSSL_INCLUDE_DIR=%cd%\..\..\..\..\..\third_party\openssl\win\%USE_PLATFORM%\include -DOPENSSL_CRYPTO_LIBRARY=%cd%\..\..\..\..\..\third_party\openssl\win\%USE_PLATFORM%\lib\libcrypto.lib ..\..
	if %errorlevel% neq 0 (
		echo "CMake failed to generate the Release version of Curl !"
		goto endlocal
	)
	ninja -j %NUMBER_OF_PROCESSORS%
	if %errorlevel% neq 0 (
		echo "Compiling the Release version of Curl failed !"
		goto endlocal
	)
	ninja install
	if %errorlevel% neq 0 (
		echo "Failed to install the Release version of Curl !"
		goto endlocal
	)
	if %USE_PLATFORM% == x64 (
		if not exist ..\..\..\..\..\third_party\curl\win\%USE_PLATFORM%\bin\libcrypto-1_1-x64.dll (
			copy ..\..\..\..\..\third_party\openssl\win\%USE_PLATFORM%\bin\libcrypto-1_1-x64.dll ..\..\..\..\..\third_party\curl\win\%USE_PLATFORM%\bin /y
		)
		if not exist ..\..\..\..\..\third_party\curl\win\%USE_PLATFORM%\bin\libssl-1_1-x64.dll (
			copy ..\..\..\..\..\third_party\openssl\win\%USE_PLATFORM%\bin\libssl-1_1-x64.dll ..\..\..\..\..\third_party\curl\win\%USE_PLATFORM%\bin /y
		)
	) else (
		if not exist ..\..\..\..\..\third_party\curl\win\%USE_PLATFORM%\bin\libcrypto-1_1.dll (
			copy ..\..\..\..\..\third_party\openssl\win\%USE_PLATFORM%\bin\libcrypto-1_1.dll ..\..\..\..\..\third_party\curl\win\%USE_PLATFORM%\bin /y
		)
		if not exist ..\..\..\..\..\third_party\curl\win\%USE_PLATFORM%\bin\libssl-1_1.dll (
			copy ..\..\..\..\..\third_party\openssl\win\%USE_PLATFORM%\bin\libssl-1_1.dll ..\..\..\..\..\third_party\curl\win\%USE_PLATFORM%\bin /y
		)
	)
	cd ..\..\..\..\..
	rd /s /q submodule\win\curl\cbuild
)

@rem install protobuf
if %CLEAR_SUBMODULE% == yes (
    if exist submodule\win\protobuf rd /s /q submodule\win\protobuf 
    if exist third_party\protobuf\win\%USE_PLATFORM% rd /s /q third_party\protobuf\win\%USE_PLATFORM%
    if exist .git\modules\submodule\win\protobuf rd /s /q .git\modules\submodule\win\protobuf
)
if not exist "third_party\protobuf\win\%USE_PLATFORM%\lib\libprotobuf.lib" (
	git submodule add -f http://github.com/protocolbuffers/protobuf.git submodule/win/protobuf
	if %errorlevel% neq 0 (
		echo "Git add submodule protobuf failed !"
		goto endlocal
	) 
	cd submodule\win\protobuf 
	git checkout v3.11.4
	if %errorlevel% neq 0 (
		echo "Git checkout protobuf-v3.11.4 failed !"
		goto endlocal
	)
	git submodule update --init
	if %errorlevel% neq 0 (
		echo "Git update submodule Protobuf failed !"
		goto endlocal
	)
	cd cmake 
	if not exist cbuild (
		mkdir cbuild
	)
	cd cbuild
	@rem build Debug
	if not exist Debug (
		mkdir Debug
		cd Debug
	) else (
		cd Debug
		del /q *
	)
	cmake -G "Ninja" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -Dprotobuf_BUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=..\..\..\..\..\..\third_party\protobuf\win\%USE_PLATFORM% ..\..
	if %errorlevel% neq 0 (
		echo "CMake failed to generate the Debug version of protobuf !"
		goto endlocal
	)
	ninja -j %NUMBER_OF_PROCESSORS%
	if %errorlevel% neq 0 (
		echo "Compiling the Debug version of protobuf failed !"
		goto endlocal
	)
	ninja install
	if %errorlevel% neq 0 (
		echo "Failed to install the Debug version of protobuf !"
		goto endlocal
	)
	cd ..
	@rem build Release
	if not exist Release (
		mkdir Release
		cd Release
	) else (
		cd Release
		del /q *
	)
	cmake -G "Ninja" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -Dprotobuf_BUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=..\..\..\..\..\..\third_party\protobuf\win\%USE_PLATFORM% ..\..
	if %errorlevel% neq 0 (
		echo "CMake failed to generate the Release version of protobuf !"
		goto endlocal
	)
	ninja -j %NUMBER_OF_PROCESSORS%
	if %errorlevel% neq 0 (
		echo "Compiling the Release version of protobuf failed !"
		goto endlocal
	)
	ninja install
	if %errorlevel% neq 0 (
		echo "Failed to install the Release version of protobuf !"
		goto endlocal
	)
	cd ..\..\..\..\..\..
	rd /s /q submodule\win\protobuf\cmake\cbuild
)

@rem install zeromq 
if %CLEAR_SUBMODULE% == yes (
    if exist submodule\win\zeromq rd /s /q submodule\win\zeromq
    if exist third_party\zeromq\win\%USE_PLATFORM% rd /s /q third_party\zeromq\win\%USE_PLATFORM%
    if exist .git\modules\submodule\win\zeromq rd /s /q .git\modules\submodule\win\zeromq
)
if not exist "third_party\zeromq\win\%USE_PLATFORM%\lib\libzmq-mt-4_0_10.lib" (
	git submodule add -f https://github.com/zeromq/zeromq4-x.git submodule/win/zeromq
	if %errorlevel% neq 0 (
		echo "Git add submodule zeromq failed !"
		goto endlocal
	)
	cd submodule\win\zeromq
	git checkout v4.0.10
	if %errorlevel% neq 0 (
		echo "Git checkout zeromq-v4.0.10 failed !"
		goto endlocal
	)
	git submodule update --init
	if %errorlevel% neq 0 (
		echo "Git update submodule ZeroMQ failed !"
		goto endlocal
	)
	if not exist cbuild (
		mkdir cbuild
	)
	cd cbuild
	@rem build Debug
	if not exist Debug (
		mkdir Debug
		cd Debug
	) else (
		cd Debug
		del /q *
	)
	cmake -G "Ninja" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=..\..\..\..\..\third_party\zeromq\win\%USE_PLATFORM% ..\..
	if %errorlevel% neq 0 (
		echo "CMake failed to generate the Debug version of ZeroMQ !"
		goto endlocal
	)
	ninja -j %NUMBER_OF_PROCESSORS%
	if %errorlevel% neq 0 (
		echo "Compiling the Debug version of ZeroMQ failed !"
		goto endlocal
	)
	ninja install
	if %errorlevel% neq 0 (
		echo "Failed to install the Debug version of ZeroMQ !"
		goto endlocal
	)
	cd ..
	@rem build Release
	if not exist Release (
		mkdir Release
		cd Release
	) else (
		cd Release
		del /q *
	)
	cmake -G "Ninja" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=..\..\..\..\..\third_party\zeromq\win\%USE_PLATFORM% ..\..
	if %errorlevel% neq 0 (
		echo "CMake failed to generate the Release version of ZeroMQ !"
		goto endlocal
	)
	ninja -j %NUMBER_OF_PROCESSORS%
	if %errorlevel% neq 0 (
		echo "Compiling the Release version of ZeroMQ failed !"
		goto endlocal
	)
	ninja install
	if %errorlevel% neq 0 (
		echo "Failed to install the Release version of ZeroMQ !"
		goto endlocal
	)
	cd ..\..\..\..\..
	rd /s /q submodule\win\zeromq\cbuild
)
@rem install cppzmq 
if %CLEAR_SUBMODULE% == yes (
    if exist submodule\win\cppzmq rd /s /q submodule\win\cppzmq
    if exist third_party\cppzmq rd /s /q third_party\cppzmq
    if exist .git\modules\submodule\win\cppzmq rd /s /q .git\modules\submodule\win\cppzmq
)
if not exist "third_party\cppzmq\include\zmq.hpp" (
	git submodule add -f https://github.com/zeromq/cppzmq.git submodule/win/cppzmq
	if %errorlevel% neq 0 (
	    echo "Git add submodule cppzmq failed !"
		goto endlocal
	)
	cd submodule/win/cppzmq
	git checkout v4.2.2
	if %errorlevel% neq 0 (
	    echo "Git checkout cppzmq-v4.2.2 failed !"
		goto endlocal
	)
	git submodule update --init
	if %errorlevel% neq 0 (
	    echo "Git update submodule cppzmq failed !"
		goto endlocal
	)
	mkdir ..\..\..\third_party\cppzmq
	mkdir ..\..\..\third_party\cppzmq\include
	copy /y zmq.hpp ..\..\..\third_party\cppzmq\include\zmq.hpp
	cd ..\..\..
)

@rem install mysql 
if %CLEAR_SUBMODULE% == yes (
    if exist submodule\win\mysql rd /s /q submodule\win\mysql 
    if exist third_party\mysql\win\%USE_PLATFORM% rd /s /q third_party\mysql\win\%USE_PLATFORM%
)

cd submodule\win
if not exist mysql (
	mkdir mysql
)
cd mysql

if not exist bison.zip (
	powershell -Command "Invoke-WebRequest -Uri 'https://raw-cdn.gitcode.com/open-source-toolkit/2bc51/blobs/1cd2e1b4cffe0a026aecbb4503de15800efeedd5/bison.zip' -OutFile 'bison.zip'"
	if %errorlevel% neq 0 (
	    echo "Downloading bison.zip failed !"
		goto endlocal
	)
)
if not exist bison (
	powershell -Command "Expand-Archive %cd%\bison.zip -Force -DestinationPath %cd%"
	if %errorlevel% neq 0 (
	    echo "Unzipping bison.zip failed !"
		goto endlocal
	)
)
set PATH=%PATH%;%cd%\bison\bin
set BISON_PKGDATADIR=%cd%\bison\data
set M4=%cd%\bison\bin\m4.exe

if exist mysql-8.0.43.zip (
	for /f "skip=1 delims=" %%i in ('certutil -hashfile "mysql-8.0.43.zip" MD5') do (
	    set "fileMD5=%%i"
		goto :compare
	)
	:compare
	set "fileMD5=%fileMD5: =%"
	if "%fileMD5%" == "1a04cee2c1d3836aed68471d82ae5942" (
	    echo "MySQL MD5 matching"
    ) else (
        echo "Mysql MD5 mismatch"
		del /q mysql-8.0.43.zip
    )
)

if not exist mysql-8.0.43.zip (
    powershell -Command "Invoke-WebRequest -Uri 'https://cdn.mysql.com//Downloads/MySQL-8.0/mysql-8.0.43.zip' -OutFile 'mysql-8.0.43.zip'"
    if %errorlevel% neq 0 (
        echo "Downloading mysql-8.0.43.zip failed !"
		echo "If the download fails, you can try downloading it from the browser using the address: https://cdn.mysql.com//Downloads/MySQL-8.0/mysql-8.0.43.zip and copy it to the directory: %cd%"
	    goto endlocal
    )
)
if not exist mysql-8.0.43 (
    powershell -Command "Expand-Archive %cd%\mysql-8.0.43.zip -Force -DestinationPath %cd%"
    if %errorlevel% neq 0 (
        echo "Unzipping mysql-8.0.43.zip failed !"
	    goto endlocal
    )
)
@rem del /f /a /q mysql-8.0.43.zip
cd mysql-8.0.43
@rem set sql_locale.cc utf8 BOM -
if exist sql\sql_locale.cc (
	powershell -c "[System.IO.FileInfo] $file = Get-Item -Path %cd%\sql\sql_locale.cc ; $sequenceBOM = New-Object System.Byte[] 3  ; $reader = $file.OpenRead(); $bytesRead = $reader.Read($sequenceBOM, 0, 3); $reader.Dispose(); if ($bytesRead -ne 3 -or $sequenceBOM[0] -ne 239 -or $sequenceBOM[1] -ne 187 -or $sequenceBOM[2] -ne 191) { [io.file]::WriteAllText('sql_locale.cc.temp','',[System.Text.Encoding]::UTF8) } else { echo 'sql_locale.cc already BOM' } "
	if exist sql_locale.cc.temp (
		type "sql\sql_locale.cc">>"sql_locale.cc.temp"
		move /y "sql_locale.cc.temp" "sql\sql_locale.cc" >nul
	)
)
@rem set my_alloc.h utf8 BOM -
if exist include\my_alloc.h (
	powershell -c "[System.IO.FileInfo] $file = Get-Item -Path %cd%\include\my_alloc.h ; $sequenceBOM = New-Object System.Byte[] 3  ; $reader = $file.OpenRead(); $bytesRead = $reader.Read($sequenceBOM, 0, 3); $reader.Dispose(); if ($bytesRead -ne 3 -or $sequenceBOM[0] -ne 239 -or $sequenceBOM[1] -ne 187 -or $sequenceBOM[2] -ne 191) { [io.file]::WriteAllText('my_alloc.h.temp','',[System.Text.Encoding]::UTF8) } else { echo 'my_alloc.h already BOM' } "
	if exist my_alloc.h.temp (
		type "include\my_alloc.h">>"my_alloc.h.temp"
		move /y "my_alloc.h.temp" "include\my_alloc.h" >nul
	)
)
@rem set handler.h utf8 BOM -
if exist sql\handler.h (
	powershell -c "[System.IO.FileInfo] $file = Get-Item -Path %cd%\sql\handler.h ; $sequenceBOM = New-Object System.Byte[] 3  ; $reader = $file.OpenRead(); $bytesRead = $reader.Read($sequenceBOM, 0, 3); $reader.Dispose(); if ($bytesRead -ne 3 -or $sequenceBOM[0] -ne 239 -or $sequenceBOM[1] -ne 187 -or $sequenceBOM[2] -ne 191) { [io.file]::WriteAllText('handler.h.temp','',[System.Text.Encoding]::UTF8) } else { echo 'handler.h already BOM' } "
	if exist handler.h.temp (
		type "sql\handler.h">>"handler.h.temp"
		move /y "handler.h.temp" "sql\handler.h" >nul
	)
)
@rem set mem_root_deque.h utf8 BOM -
if exist include\mem_root_deque.h (
	powershell -c "[System.IO.FileInfo] $file = Get-Item -Path %cd%\include\mem_root_deque.h ; $sequenceBOM = New-Object System.Byte[] 3  ; $reader = $file.OpenRead(); $bytesRead = $reader.Read($sequenceBOM, 0, 3); $reader.Dispose(); if ($bytesRead -ne 3 -or $sequenceBOM[0] -ne 239 -or $sequenceBOM[1] -ne 187 -or $sequenceBOM[2] -ne 191) { [io.file]::WriteAllText('mem_root_deque.h.temp','',[System.Text.Encoding]::UTF8) } else { echo 'mem_root_deque.h already BOM' } "
	if exist mem_root_deque.h.temp (
		type "include\mem_root_deque.h">>"mem_root_deque.h.temp"
		move /y "mem_root_deque.h.temp" "include\mem_root_deque.h" >nul
	)
)
@rem set table.h utf8 BOM -
if exist sql\table.h (
	powershell -c "[System.IO.FileInfo] $file = Get-Item -Path %cd%\sql\table.h ; $sequenceBOM = New-Object System.Byte[] 3  ; $reader = $file.OpenRead(); $bytesRead = $reader.Read($sequenceBOM, 0, 3); $reader.Dispose(); if ($bytesRead -ne 3 -or $sequenceBOM[0] -ne 239 -or $sequenceBOM[1] -ne 187 -or $sequenceBOM[2] -ne 191) { [io.file]::WriteAllText('table.h.temp','',[System.Text.Encoding]::UTF8) } else { echo 'table.h already BOM' } "
	if exist table.h.temp (
		type "sql\table.h">>"table.h.temp"
		move /y "table.h.temp" "sql\table.h" >nul
	)
)
@rem set field.h utf8 BOM -
if exist sql\field.h (
	powershell -c "[System.IO.FileInfo] $file = Get-Item -Path %cd%\sql\field.h ; $sequenceBOM = New-Object System.Byte[] 3  ; $reader = $file.OpenRead(); $bytesRead = $reader.Read($sequenceBOM, 0, 3); $reader.Dispose(); if ($bytesRead -ne 3 -or $sequenceBOM[0] -ne 239 -or $sequenceBOM[1] -ne 187 -or $sequenceBOM[2] -ne 191) { [io.file]::WriteAllText('field.h.temp','',[System.Text.Encoding]::UTF8) } else { echo 'field.h already BOM' } "
	if exist field.h.temp (
		type "sql\field.h">>"field.h.temp"
		move /y "field.h.temp" "sql\field.h" >nul
	)
)
@rem set item.h utf8 BOM -
if exist sql\item.h (
	powershell -c "[System.IO.FileInfo] $file = Get-Item -Path %cd%\sql\item.h ; $sequenceBOM = New-Object System.Byte[] 3  ; $reader = $file.OpenRead(); $bytesRead = $reader.Read($sequenceBOM, 0, 3); $reader.Dispose(); if ($bytesRead -ne 3 -or $sequenceBOM[0] -ne 239 -or $sequenceBOM[1] -ne 187 -or $sequenceBOM[2] -ne 191) { [io.file]::WriteAllText('item.h.temp','',[System.Text.Encoding]::UTF8) } else { echo 'item.h already BOM' } "
	if exist item.h.temp (
		type "sql\item.h">>"item.h.temp"
		move /y "item.h.temp" "sql\item.h" >nul
	)
)
@rem set row_iterator.h utf8 BOM -
if exist sql\iterators\row_iterator.h (
	powershell -c "[System.IO.FileInfo] $file = Get-Item -Path %cd%\sql\iterators\row_iterator.h ; $sequenceBOM = New-Object System.Byte[] 3  ; $reader = $file.OpenRead(); $bytesRead = $reader.Read($sequenceBOM, 0, 3); $reader.Dispose(); if ($bytesRead -ne 3 -or $sequenceBOM[0] -ne 239 -or $sequenceBOM[1] -ne 187 -or $sequenceBOM[2] -ne 191) { [io.file]::WriteAllText('row_iterator.h.temp','',[System.Text.Encoding]::UTF8) } else { echo 'row_iterator.h already BOM' } "
	if exist row_iterator.h.temp (
		type "sql\iterators\row_iterator.h">>"row_iterator.h.temp"
		move /y "row_iterator.h.temp" "sql\iterators\row_iterator.h" >nul
	)
)
@rem set item_func.h utf8 BOM -
if exist sql\item_func.h (
	powershell -c "[System.IO.FileInfo] $file = Get-Item -Path %cd%\sql\item_func.h ; $sequenceBOM = New-Object System.Byte[] 3  ; $reader = $file.OpenRead(); $bytesRead = $reader.Read($sequenceBOM, 0, 3); $reader.Dispose(); if ($bytesRead -ne 3 -or $sequenceBOM[0] -ne 239 -or $sequenceBOM[1] -ne 187 -or $sequenceBOM[2] -ne 191) { [io.file]::WriteAllText('item_func.h.temp','',[System.Text.Encoding]::UTF8) } else { echo 'item_func.h already BOM' } "
	if exist item_func.h.temp (
		type "sql\item_func.h">>"item_func.h.temp"
		move /y "item_func.h.temp" "sql\item_func.h" >nul
	)
)
@rem set sql_select.h utf8 BOM -
if exist sql\sql_select.h (
	powershell -c "[System.IO.FileInfo] $file = Get-Item -Path %cd%\sql\sql_select.h ; $sequenceBOM = New-Object System.Byte[] 3  ; $reader = $file.OpenRead(); $bytesRead = $reader.Read($sequenceBOM, 0, 3); $reader.Dispose(); if ($bytesRead -ne 3 -or $sequenceBOM[0] -ne 239 -or $sequenceBOM[1] -ne 187 -or $sequenceBOM[2] -ne 191) { [io.file]::WriteAllText('sql_select.h.temp','',[System.Text.Encoding]::UTF8) } else { echo 'sql_select.h already BOM' } "
	if exist sql_select.h.temp (
		type "sql\sql_select.h">>"sql_select.h.temp"
		move /y "sql_select.h.temp" "sql\sql_select.h" >nul
	)
)
@rem set sql_lex.h utf8 BOM -
if exist sql\sql_lex.h (
	powershell -c "[System.IO.FileInfo] $file = Get-Item -Path %cd%\sql\sql_lex.h ; $sequenceBOM = New-Object System.Byte[] 3  ; $reader = $file.OpenRead(); $bytesRead = $reader.Read($sequenceBOM, 0, 3); $reader.Dispose(); if ($bytesRead -ne 3 -or $sequenceBOM[0] -ne 239 -or $sequenceBOM[1] -ne 187 -or $sequenceBOM[2] -ne 191) { [io.file]::WriteAllText('sql_lex.h.temp','',[System.Text.Encoding]::UTF8) } else { echo 'sql_lex.h already BOM' } "
	if exist sql_lex.h.temp (
		type "sql\sql_lex.h">>"sql_lex.h.temp"
		move /y "sql_lex.h.temp" "sql\sql_lex.h" >nul
	)
)
@rem set test_string_service_charset.cc  utf8 BOM -
if exist components\example\test_string_service_charset.cc (
	powershell -c "[System.IO.FileInfo] $file = Get-Item -Path %cd%\components\example\test_string_service_charset.cc ; $sequenceBOM = New-Object System.Byte[] 3  ; $reader = $file.OpenRead(); $bytesRead = $reader.Read($sequenceBOM, 0, 3); $reader.Dispose(); if ($bytesRead -ne 3 -or $sequenceBOM[0] -ne 239 -or $sequenceBOM[1] -ne 187 -or $sequenceBOM[2] -ne 191) { [io.file]::WriteAllText('test_string_service_charset.cc.temp','',[System.Text.Encoding]::UTF8) } else { echo 'test_string_service_charset.cc already BOM' } "
	if exist test_string_service_charset.cc.temp (
		type "components\example\test_string_service_charset.cc">>"test_string_service_charset.cc.temp"
		move /y "test_string_service_charset.cc.temp" "components\example\test_string_service_charset.cc" >nul
	)
)
@rem set strings_utf8-t.cc utf8 BOM -
if exist unittest\gunit\strings_utf8-t.cc (
	powershell -c "[System.IO.FileInfo] $file = Get-Item -Path %cd%\unittest\gunit\strings_utf8-t.cc ; $sequenceBOM = New-Object System.Byte[] 3  ; $reader = $file.OpenRead(); $bytesRead = $reader.Read($sequenceBOM, 0, 3); $reader.Dispose(); if ($bytesRead -ne 3 -or $sequenceBOM[0] -ne 239 -or $sequenceBOM[1] -ne 187 -or $sequenceBOM[2] -ne 191) { [io.file]::WriteAllText('strings_utf8-t.cc.temp','',[System.Text.Encoding]::UTF8) } else { echo 'strings_utf8-t.cc already BOM' } "
	if exist strings_utf8-t.cc.temp (
		type "unittest\gunit\strings_utf8-t.cc">>"strings_utf8-t.cc.temp"
		move /y "strings_utf8-t.cc.temp" "unittest\gunit\strings_utf8-t.cc" >nul
	)
)
@rem set strings_strnxfrm-t.cc utf8 BOM -
if exist unittest\gunit\strings_strnxfrm-t.cc (
	powershell -c "[System.IO.FileInfo] $file = Get-Item -Path %cd%\unittest\gunit\strings_strnxfrm-t.cc ; $sequenceBOM = New-Object System.Byte[] 3  ; $reader = $file.OpenRead(); $bytesRead = $reader.Read($sequenceBOM, 0, 3); $reader.Dispose(); if ($bytesRead -ne 3 -or $sequenceBOM[0] -ne 239 -or $sequenceBOM[1] -ne 187 -or $sequenceBOM[2] -ne 191) { [io.file]::WriteAllText('strings_strnxfrm-t.cc.temp','',[System.Text.Encoding]::UTF8) } else { echo 'strings_strnxfrm-t.cc already BOM' } "
	if exist strings_strnxfrm-t.cc.temp (
		type "unittest\gunit\strings_strnxfrm-t.cc">>"strings_strnxfrm-t.cc.temp"
		move /y "strings_strnxfrm-t.cc.temp" "unittest\gunit\strings_strnxfrm-t.cc" >nul
	)
)
@rem set strings_valid_check-t.cc utf8 BOM -
if exist unittest\gunit\strings_valid_check-t.cc (
	powershell -c "[System.IO.FileInfo] $file = Get-Item -Path %cd%\unittest\gunit\strings_valid_check-t.cc ; $sequenceBOM = New-Object System.Byte[] 3  ; $reader = $file.OpenRead(); $bytesRead = $reader.Read($sequenceBOM, 0, 3); $reader.Dispose(); if ($bytesRead -ne 3 -or $sequenceBOM[0] -ne 239 -or $sequenceBOM[1] -ne 187 -or $sequenceBOM[2] -ne 191) { [io.file]::WriteAllText('strings_valid_check-t.cc.temp','',[System.Text.Encoding]::UTF8) } else { echo 'strings_valid_check-t.cc already BOM' } "
	if exist strings_valid_check-t.cc.temp (
		type "unittest\gunit\strings_valid_check-t.cc">>"strings_valid_check-t.cc.temp"
		move /y "strings_valid_check-t.cc.temp" "unittest\gunit\strings_valid_check-t.cc" >nul
	)
)
copy /y ..\..\..\boost.cmake cmake\boost.cmake
copy /y ..\..\..\buffer.cc sql\gis\buffer.cc
if not exist cbuild (
	mkdir cbuild
)
cd cbuild
@rem build Release
if not exist Release (
	mkdir Release
	cd Release
) else (
	cd Release
	del /q *
)
cmake -G "Ninja" -DWITHOUT_SERVER=1 -DCMAKE_BUILD_TYPE=Relwithdebinfo -DCMAKE_INSTALL_PREFIX=..\..\..\..\..\..\third_party\mysql\win\%USE_PLATFORM% -DBOOST_INCLUDE_DIR=..\..\..\..\..\..\third_party\boost\win\%USE_PLATFORM%\include\boost-1_78 -DUSING_SYSTEM_BOOST=OFF -DLOCAL_BOOST_DIR=..\..\..\..\..\..\third_party\boost\win\%USE_PLATFORM% -DOPENSSL_ROOT_DIR=%cd%\..\..\..\..\..\..\third_party\openssl\win\%USE_PLATFORM%\ -DOPENSSL_INCLUDE_DIR=..\..\..\..\..\..\third_party\openssl\win\%USE_PLATFORM%\include -DOPENSSL_LIBRARY=..\..\..\..\..\..\third_party\openssl\win\%USE_PLATFORM%\lib\libssl.lib -DCRYPTO_LIBRARY=..\..\..\..\..\..\third_party\openssl\win\%USE_PLATFORM%\lib\libcrypto.lib ..\..
if %errorlevel% neq 0 (
    echo "CMake failed to generate the Release version of mysql !"
	goto endlocal
)
if %USE_PLATFORM% == x64 (
	if not exist extra\libcrypto-1_1-x64.dll (
		copy ..\..\..\..\..\..\third_party\openssl\win\%USE_PLATFORM%\bin\libcrypto-1_1-x64.dll extra /y
	)
	if not exist libmysql\libcrypto-1_1-x64.dll (
		copy ..\..\..\..\..\..\third_party\openssl\win\%USE_PLATFORM%\bin\libcrypto-1_1-x64.dll libmysql /y
	)
	if not exist libmysql\libssl-1_1-x64.dll (
		copy ..\..\..\..\..\..\third_party\openssl\win\%USE_PLATFORM%\bin\libssl-1_1-x64.dll libmysql /y
	)
) else (
	if not exist extra\libcrypto-1_1.dll (
		copy ..\..\..\..\..\..\third_party\openssl\win\%USE_PLATFORM%\bin\libcrypto-1_1.dll extra /y
	)
	if not exist libmysql\libcrypto-1_1.dll (
		copy ..\..\..\..\..\..\third_party\openssl\win\%USE_PLATFORM%\bin\libcrypto-1_1.dll libmysql /y
	)
	if not exist libmysql\libssl-1_1.dll (
		copy ..\..\..\..\..\..\third_party\openssl\win\%USE_PLATFORM%\bin\libssl-1_1.dll libmysql /y
	)
)

ninja -j %NUMBER_OF_PROCESSORS%
rem sql_commands_help_data.h error C2001: newline in constant
if %errorlevel% neq 0 (
	rem set sql_commands_help_data.h utf8 BOM
	if exist scripts\sql_commands_help_data.h (
		powershell -c "[System.IO.FileInfo] $file = Get-Item -Path %cd%\scripts\sql_commands_help_data.h ; $sequenceBOM = New-Object System.Byte[] 3  ; $reader = $file.OpenRead(); $bytesRead = $reader.Read($sequenceBOM, 0, 3); $reader.Dispose(); if ($bytesRead -ne 3 -or $sequenceBOM[0] -ne 239 -or $sequenceBOM[1] -ne 187 -or $sequenceBOM[2] -ne 191) { [io.file]::WriteAllText('sql_commands_help_data.h.temp','',[System.Text.Encoding]::UTF8) } else { echo echo 'sql_commands_help_data.h already BOM' } "
		if exist sql_commands_help_data.h.temp (
			type "scripts\sql_commands_help_data.h">>"sql_commands_help_data.h.temp"
			move /y "sql_commands_help_data.h.temp" "scripts\sql_commands_help_data.h" >nul
		)
	)
	ninja -j %NUMBER_OF_PROCESSORS%
	if %errorlevel% neq 0 (
	    echo "Recompiling MySQL failed !"
		goto endlocal
	)
)
ninja install
if %errorlevel% neq 0 (
    echo "Installation of MySQL failed !"
	goto endlocal
)
cd ..\..\..\..\..\..
rd /s /q submodule\win\mysql\mysql-8.0.43\cbuild


:endlocal
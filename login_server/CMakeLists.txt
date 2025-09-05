cmake_minimum_required(VERSION 3.5)

#############################################
# CPU architecture
if(NOT DEFINED CMAKE_SYSTEM_PROCESSOR)
    set(CMAKE_SYSTEM_PROCESSOR "x86_64") 
endif()
# Processor architecture name
if(NOT MSVC)
    set(USE_PLATFORM ${CMAKE_SYSTEM_PROCESSOR})
endif()
# Project Name
set(CONFIG_PROJECT_NAME "LoginServer")
# Version number
set(${CONFIG_PROJECT_NAME}_VERSION_MAJOR 1)
set(${CONFIG_PROJECT_NAME}_VERSION_MINOR 0)
###############################################

project(${CONFIG_PROJECT_NAME})

if(MSVC)
	# cmake -G "Visual Studio 17 2022" -T v143 ..
	# cmake -G "Visual Studio 17 2022" -T v143 -A Win32 ..
	if(MSVC_VERSION LESS 1930)
		message(FATAL_ERROR "You need at least Visual Studio 17 (Microsoft Visual Studio 2022)")
	endif()
	
	if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
		set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin/win/x64/$(Configuration))
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/lib/win/x64/Debug)
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin/win/x64/Debug)
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/lib/win/x64/Release)
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/bin/win/x64/Release)
		
	elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Win32")
		set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin/win/x32/$(Configuration))
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/lib/win/x32/Debug)
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin/win/x32/Debug)
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/lib/win/x32/Release)
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/bin/win/x32/Release)
		
	endif()
else()
	# cmake -G "Unix Makefiles" -DCMAKE_SYSTEM_PROCESSOR=x86_64 -DCMAKE_BUILD_TYPE=Debug ..
	# cmake -G "Unix Makefiles" -DCMAKE_SYSTEM_PROCESSOR=x86_64 -DCMAKE_BUILD_TYPE=Release ..
	set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin/linux/${USE_PLATFORM}/${CMAKE_BUILD_TYPE})
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/${USE_PLATFORM}/Debug)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin/linux/${USE_PLATFORM}/Debug)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/${USE_PLATFORM}/Release)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/bin/linux/${USE_PLATFORM}/Release)
endif()

# Compile as C++11
add_definitions(-std=c++11)
add_definitions(-D_SCL_SECURE_NO_WARNINGS)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-DHAVE_STDCXX_0X)

if(MSVC)
    add_definitions(-DSHARE_DLL)
    add_definitions(-DPROTOBUF_USE_DLLS)
else()
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_definitions(-D_DEBUG)
    elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
        add_definitions(-D_RELEASE)
    endif()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -Wall -g -ggdb")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -Wall")
endif()


# Find all source files in the framework directory
# And save the name to the DIR_SRCS variable
aux_source_directory(src DIR_SRCS)
aux_source_directory(src/framework DIR_SRCS)
aux_source_directory(src/memcache DIR_SRCS)
aux_source_directory(src/template DIR_SRCS)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/json DIR_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/rpcz DIR_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/tinyxml DIR_SRCS)

# Add header file path
include_directories(src)
include_directories(src/framework)
include_directories(src/template)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../shared/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../shared/include/utility)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../shared/include/threading)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../shared/include/timerevent)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../shared/include/module)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../shared/include/network)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../shared/include/httpclient)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/cppzmq/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/rpcz)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/json)


if(MSVC)

if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/boost/win/x64/include/boost-1_78)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/protobuf/win/x64/include)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/zeromq/win/x64/include)
	
	# Associated library file directory
	link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/boost/win/x64/lib)
	link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/protobuf/win/x64/lib)
	link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/zeromq/win/x64/lib)
	link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../shared/lib/win/x64)
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Win32")
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/boost/win/x32/include/boost-1_78)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/protobuf/win/x32/include)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/zeromq/win/x32/include)
	
	# Associated library file directory
	link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/boost/win/x32/lib)
	link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/protobuf/win/x32/lib)
	link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/zeromq/win/x32/lib)
	link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../shared/lib/win/x32)
endif()


else()
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/boost/linux/${USE_PLATFORM}/include)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/protobuf/linux/${USE_PLATFORM}/include)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/zeromq/linux/${USE_PLATFORM}/include)
	
	# Associated library file directory
	link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/boost/linux/${USE_PLATFORM}/lib)
	link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/protobuf/linux/${USE_PLATFORM}/lib)
	link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/zeromq/linux/${USE_PLATFORM}/lib)
	link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../shared/lib/linux/${USE_PLATFORM}/${CMAKE_BUILD_TYPE})

endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake_modules/functions.cmake)

group_add_executable(${CONFIG_PROJECT_NAME} ${DIR_SRCS})

if(MSVC)
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
		target_link_libraries(${CONFIG_PROJECT_NAME} debug libboost_chrono-vc143-mt-gd-x64-1_78)
		target_link_libraries(${CONFIG_PROJECT_NAME} debug libboost_date_time-vc143-mt-gd-x64-1_78)
		target_link_libraries(${CONFIG_PROJECT_NAME} debug libboost_system-vc143-mt-gd-x64-1_78)
		target_link_libraries(${CONFIG_PROJECT_NAME} debug libboost_thread-vc143-mt-gd-x64-1_78)
	elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Win32")
		target_link_libraries(${CONFIG_PROJECT_NAME} debug libboost_chrono-vc143-mt-gd-x32-1_78)
		target_link_libraries(${CONFIG_PROJECT_NAME} debug libboost_date_time-vc143-mt-gd-x32-1_78)
		target_link_libraries(${CONFIG_PROJECT_NAME} debug libboost_system-vc143-mt-gd-x32-1_78)
		target_link_libraries(${CONFIG_PROJECT_NAME} debug libboost_thread-vc143-mt-gd-x32-1_78)
	endif()
	target_link_libraries(${CONFIG_PROJECT_NAME} debug libprotobufd)
	target_link_libraries(${CONFIG_PROJECT_NAME} debug libzmq-mt-gd-4_0_10)
	target_link_libraries(${CONFIG_PROJECT_NAME} debug shared)

    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
		target_link_libraries(${CONFIG_PROJECT_NAME} optimized libboost_chrono-vc143-mt-x64-1_78)
		target_link_libraries(${CONFIG_PROJECT_NAME} optimized libboost_date_time-vc143-mt-x64-1_78)
		target_link_libraries(${CONFIG_PROJECT_NAME} optimized libboost_system-vc143-mt-x64-1_78)
		target_link_libraries(${CONFIG_PROJECT_NAME} optimized libboost_thread-vc143-mt-x64-1_78)
	elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Win32")
		target_link_libraries(${CONFIG_PROJECT_NAME} optimized libboost_chrono-vc143-mt-x32-1_78)
		target_link_libraries(${CONFIG_PROJECT_NAME} optimized libboost_date_time-vc143-mt-x32-1_78)
		target_link_libraries(${CONFIG_PROJECT_NAME} optimized libboost_system-vc143-mt-x32-1_78)
		target_link_libraries(${CONFIG_PROJECT_NAME} optimized libboost_thread-vc143-mt-x32-1_78)
	endif()
	target_link_libraries(${CONFIG_PROJECT_NAME} optimized libprotobuf)
	target_link_libraries(${CONFIG_PROJECT_NAME} optimized libzmq-mt-4_0_10)
	target_link_libraries(${CONFIG_PROJECT_NAME} optimized shared)

    # Set run environment
	if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
		add_custom_command(TARGET ${CONFIG_PROJECT_NAME} POST_BUILD
			COMMAND xcopy $(ProjectDir)..\\src\\App.config $(TargetDir) /y
			COMMAND xcopy $(ProjectDir)..\\src\\templatefiles.csv $(TargetDir) /y
			COMMAND xcopy $(ProjectDir)..\\src\\resource $(TargetDir)resource /y /e /i
			COMMAND if $<CONFIG:Debug> == 1 xcopy $(ProjectDir)..\\..\\shared\\bin\\win\\x64\\Debug\\Shared.dll $(TargetDir) /y
			COMMAND if $<CONFIG:Release> == 1 xcopy $(ProjectDir)..\\..\\shared\\bin\\win\\x64\\Release\\Shared.dll $(TargetDir) /y
			COMMAND if $<CONFIG:Debug> == 1 xcopy $(ProjectDir)..\\..\\shared\\bin\\win\\x64\\Debug\\Shared.pdb $(TargetDir) /y
			COMMAND if $<CONFIG:Release> == 1 if exist $(ProjectDir)..\\..\\shared\\bin\\win\\x64\\Release\\Shared.pdb xcopy $(ProjectDir)..\\..\\shared\\bin\\win\\x64\\Release\\Shared.pdb $(TargetDir) /y
			COMMAND if $<CONFIG:Debug> == 1 xcopy $(ProjectDir)..\\..\\third_party\\curl\\win\\x64\\bin\\libcurl-d.dll $(TargetDir) /y
			COMMAND if $<CONFIG:Release> == 1 xcopy $(ProjectDir)..\\..\\third_party\\curl\\win\\x64\\bin\\libcurl.dll $(TargetDir) /y
			COMMAND xcopy $(ProjectDir)..\\..\\third_party\\mysql\\win\\x64\\lib\\libmysql.dll $(TargetDir) /y
			COMMAND xcopy $(ProjectDir)..\\..\\third_party\\mysql\\win\\x64\\lib\\libmysql.pdb $(TargetDir) /y
			COMMAND if $<CONFIG:Debug> == 1 xcopy $(ProjectDir)..\\..\\third_party\\zeromq\\win\\x64\\bin\\libzmq-mt-gd-4_0_10.dll $(TargetDir) /y
			COMMAND if $<CONFIG:Release> == 1 xcopy $(ProjectDir)..\\..\\third_party\\zeromq\\win\\x64\\bin\\libzmq-mt-4_0_10.dll $(TargetDir) /y
			COMMAND if $<CONFIG:Debug> == 1 xcopy $(ProjectDir)..\\..\\third_party\\zeromq\\win\\x64\\lib\\libzmq-mt-gd-4_0_10.pdb $(TargetDir) /y
			COMMAND if $<CONFIG:Debug> == 1 xcopy $(ProjectDir)..\\..\\third_party\\protobuf\\win\\x64\\bin\\libprotobufd.dll $(TargetDir) /y
			COMMAND if $<CONFIG:Release> == 1 xcopy $(ProjectDir)..\\..\\third_party\\protobuf\\win\\x64\\bin\\libprotobuf.dll $(TargetDir) /y
			COMMAND xcopy $(ProjectDir)..\\..\\third_party\\openssl\\win\\x64\\bin\\libcrypto-1_1-x64.dll $(TargetDir) /y
			COMMAND xcopy $(ProjectDir)..\\..\\third_party\\openssl\\win\\x64\\bin\\libcrypto-1_1-x64.pdb $(TargetDir) /y
			COMMAND xcopy $(ProjectDir)..\\..\\third_party\\openssl\\win\\x64\\bin\\libssl-1_1-x64.dll $(TargetDir) /y
			COMMAND xcopy $(ProjectDir)..\\..\\third_party\\openssl\\win\\x64\\bin\\libssl-1_1-x64.pdb $(TargetDir) /y
		)
		string(REPLACE "/" "\\" WINDOWS_CURRENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
		set_target_properties(${CONFIG_PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${WINDOWS_CURRENT_DIR}\\bin\\win\\x64\\$(Configuration))
	elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Win32")
		add_custom_command(TARGET ${CONFIG_PROJECT_NAME} POST_BUILD
			COMMAND xcopy $(ProjectDir)..\\src\\App.config $(TargetDir) /y
			COMMAND xcopy $(ProjectDir)..\\src\\templatefiles.csv $(TargetDir) /y
			COMMAND xcopy $(ProjectDir)..\\src\\resource $(TargetDir)resource /y /e /i
			COMMAND if $<CONFIG:Debug> == 1 xcopy $(ProjectDir)..\\..\\shared\\bin\\win\\x32\\Debug\\Shared.dll $(TargetDir) /y
			COMMAND if $<CONFIG:Release> == 1 xcopy $(ProjectDir)..\\..\\shared\\bin\\win\\x32\\Release\\Shared.dll $(TargetDir) /y
			COMMAND if $<CONFIG:Debug> == 1 xcopy $(ProjectDir)..\\..\\shared\\bin\\win\\x32\\Debug\\Shared.pdb $(TargetDir) /y
			COMMAND if $<CONFIG:Release> == 1 if exist $(ProjectDir)..\\..\\shared\\bin\\win\\x32\\Release\\Shared.pdb xcopy $(ProjectDir)..\\..\\shared\\bin\\win\\x32\\Release\\Shared.pdb $(TargetDir) /y
			COMMAND if $<CONFIG:Debug> == 1 xcopy $(ProjectDir)..\\..\\third_party\\curl\\win\\x32\\bin\\libcurl-d.dll $(TargetDir) /y
			COMMAND if $<CONFIG:Release> == 1 xcopy $(ProjectDir)..\\..\\third_party\\curl\\win\\x32\\bin\\libcurl.dll $(TargetDir) /y
			COMMAND xcopy $(ProjectDir)..\\..\\third_party\\mysql\\win\\x32\\lib\\libmysql.dll $(TargetDir) /y
			COMMAND xcopy $(ProjectDir)..\\..\\third_party\\mysql\\win\\x32\\lib\\libmysql.pdb $(TargetDir) /y
			COMMAND if $<CONFIG:Debug> == 1 xcopy $(ProjectDir)..\\..\\third_party\\zeromq\\win\\x32\\bin\\libzmq-mt-gd-4_0_10.dll $(TargetDir) /y
			COMMAND if $<CONFIG:Release> == 1 xcopy $(ProjectDir)..\\..\\third_party\\zeromq\\win\\x32\\bin\\libzmq-mt-4_0_10.dll $(TargetDir) /y
			COMMAND if $<CONFIG:Debug> == 1 xcopy $(ProjectDir)..\\..\\third_party\\zeromq\\win\\x32\\lib\\libzmq-mt-gd-4_0_10.pdb $(TargetDir) /y
			COMMAND if $<CONFIG:Debug> == 1 xcopy $(ProjectDir)..\\..\\third_party\\protobuf\\win\\x32\\bin\\libprotobufd.dll $(TargetDir) /y
			COMMAND if $<CONFIG:Release> == 1 xcopy $(ProjectDir)..\\..\\third_party\\protobuf\\win\\x32\\bin\\libprotobuf.dll $(TargetDir) /y
			COMMAND xcopy $(ProjectDir)..\\..\\third_party\\openssl\\win\\x32\\bin\\libcrypto-1_1.dll $(TargetDir) /y
			COMMAND xcopy $(ProjectDir)..\\..\\third_party\\openssl\\win\\x32\\bin\\libcrypto-1_1.pdb $(TargetDir) /y
			COMMAND xcopy $(ProjectDir)..\\..\\third_party\\openssl\\win\\x32\\bin\\libssl-1_1.dll $(TargetDir) /y
			COMMAND xcopy $(ProjectDir)..\\..\\third_party\\openssl\\win\\x32\\bin\\libssl-1_1.pdb $(TargetDir) /y
		)
		string(REPLACE "/" "\\" WINDOWS_CURRENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
		set_target_properties(${CONFIG_PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${WINDOWS_CURRENT_DIR}\\bin\\win\\x32\\$(Configuration))
	endif()
else()

	target_link_libraries(${CONFIG_PROJECT_NAME} pthread)

	target_link_libraries(${CONFIG_PROJECT_NAME} debug libboost_chrono.so)
	target_link_libraries(${CONFIG_PROJECT_NAME} debug libboost_date_time.so)
	target_link_libraries(${CONFIG_PROJECT_NAME} debug libboost_system.so)
	target_link_libraries(${CONFIG_PROJECT_NAME} debug libboost_thread.so)
	target_link_libraries(${CONFIG_PROJECT_NAME} debug libprotobufd.so)
	target_link_libraries(${CONFIG_PROJECT_NAME} debug libzmq.so)
	target_link_libraries(${CONFIG_PROJECT_NAME} debug libShared.so)

	target_link_libraries(${CONFIG_PROJECT_NAME} optimized libboost_chrono.so)
	target_link_libraries(${CONFIG_PROJECT_NAME} optimized libboost_date_time.so)
	target_link_libraries(${CONFIG_PROJECT_NAME} optimized libboost_system.so)
	target_link_libraries(${CONFIG_PROJECT_NAME} optimized libboost_thread.so)
	target_link_libraries(${CONFIG_PROJECT_NAME} optimized libprotobuf.so)
	target_link_libraries(${CONFIG_PROJECT_NAME} optimized libzmq.so)
	target_link_libraries(${CONFIG_PROJECT_NAME} optimized libShared.so)

	# Set run environment
	set(TARGET_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin/linux/${USE_PLATFORM}/${CMAKE_BUILD_TYPE}/)
	add_custom_command(TARGET ${CONFIG_PROJECT_NAME} POST_BUILD
		COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/src/App.config ${TARGET_DIR}
		COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/src/templatefiles.csv ${TARGET_DIR}
		COMMAND cp -rf ${CMAKE_CURRENT_SOURCE_DIR}/src/resource ${TARGET_DIR}
		COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/../shared/lib/linux/${USE_PLATFORM}/${CMAKE_BUILD_TYPE}/libShared.so ${TARGET_DIR}
		COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/mysql/linux/${USE_PLATFORM}/lib/libmysqlclient.so ${TARGET_DIR}
		COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/mysql/linux/${USE_PLATFORM}/lib/libmysqlclient.so.21 ${TARGET_DIR}
		COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/mysql/linux/${USE_PLATFORM}/lib/libmysqlclient.so.21.2.43 ${TARGET_DIR}
		COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/zeromq/linux/${USE_PLATFORM}/lib/libzmq.so ${TARGET_DIR}
		COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/zeromq/linux/${USE_PLATFORM}/lib/libzmq.so.4 ${TARGET_DIR}
		COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/zeromq/linux/${USE_PLATFORM}/lib/libzmq.so.4.0.3 ${TARGET_DIR}
	)
	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		add_custom_command(TARGET ${CONFIG_PROJECT_NAME} POST_BUILD
			COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/curl/linux/${USE_PLATFORM}/lib/libcurl-d.so ${TARGET_DIR}
			COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/protobuf/linux/${USE_PLATFORM}/lib/libprotobufd.so ${TARGET_DIR}
			COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/protobuf/linux/${USE_PLATFORM}/lib/libprotobufd.so.3.11.4.0 ${TARGET_DIR}
		)
	elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
		add_custom_command(TARGET ${CONFIG_PROJECT_NAME} POST_BUILD
			COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/curl/linux/${USE_PLATFORM}/lib/libcurl.so ${TARGET_DIR}
			COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/protobuf/linux/${USE_PLATFORM}/lib/libprotobuf.so ${TARGET_DIR}
			COMMAND cp -f ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/protobuf/linux/${USE_PLATFORM}/lib/libprotobuf.so.3.11.4.0 ${TARGET_DIR}
		)
	endif()
endif()




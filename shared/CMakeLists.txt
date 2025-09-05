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
set(CONFIG_PROJECT_NAME "Shared")
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
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/lib/win/x64/Debug)
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin/win/x64/Debug)
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/lib/win/x64/Release)
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/bin/win/x64/Release)
		
	elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Win32")
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/lib/win/x32/Debug)
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin/win/x32/Debug)
		set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/lib/win/x32/Release)
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/bin/win/x32/Release)	
	endif()
else()
	# cmake -G "Unix Makefiles" -DCMAKE_SYSTEM_PROCESSOR=x86_64 -DCMAKE_BUILD_TYPE=Debug ..
	# cmake -G "Unix Makefiles" -DCMAKE_SYSTEM_PROCESSOR=x86_64 -DCMAKE_BUILD_TYPE=Release ..
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/${USE_PLATFORM}/Debug)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/${USE_PLATFORM}/Debug)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin/linux/${USE_PLATFORM}/Debug)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/${USE_PLATFORM}/Release)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/${USE_PLATFORM}/Release)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/bin/linux/${USE_PLATFORM}/Release)
endif()

add_definitions(-D_SCL_SECURE_NO_WARNINGS)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-DHAVE_STDCXX_0X)

if(MSVC)
    add_definitions(-DSHARED_EXPORTS)
    add_definitions(-DSHARE_DLL)
else()
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_definitions(-D_DEBUG)
    elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
        add_definitions(-D_RELEASE)
    endif()
    add_definitions(-DUSE_GETTID)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -Wall -g -ggdb")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -Wall")
endif()

# Find all source files in the framework directory
# And save the name to the DIR_SRCS variable

aux_source_directory(source DIR_SRCS)
aux_source_directory(source/database DIR_SRCS)
aux_source_directory(source/httpclient DIR_SRCS)
aux_source_directory(source/module DIR_SRCS)
aux_source_directory(source/network DIR_SRCS)
aux_source_directory(source/threading DIR_SRCS)
aux_source_directory(source/timerevent DIR_SRCS)
aux_source_directory(source/utility DIR_SRCS)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/stacktrace DIR_SRCS)

# Add header file path
include_directories(include)
include_directories(include/database)
include_directories(include/httpclient)
include_directories(include/module)
include_directories(include/network)
include_directories(include/threading)
include_directories(include/timerevent)
include_directories(include/utility)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/stacktrace)


if(MSVC)
	if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
		include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/mysql/win/x64/include)
		include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/curl/win/x64/include/curl)

		# Associated library file directory
		link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/mysql/win/x64/lib)
		link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/curl/win/x64/lib)
	elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Win32")
		include_directories(../third_party/mysql/win/x32/include)
		include_directories(../third_party/curl/win/x32/include/curl)

		# Associated library file directory
		link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/mysql/win/x32/lib)
		link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/curl/win/x32/lib)
	endif()
else()
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/mysql/linux/${USE_PLATFORM}/include)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/curl/linux/${USE_PLATFORM}/include/curl)
	
	message("MYSQLpath = ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/mysql/linux/${USE_PLATFORM}/include")

    link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/mysql/linux/${USE_PLATFORM}/lib)
    link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party/curl/linux/${USE_PLATFORM}/lib)
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake_modules/functions.cmake)

group_add_library(${CONFIG_PROJECT_NAME} SHARED ${DIR_SRCS})

if(MSVC)
    target_link_libraries(${CONFIG_PROJECT_NAME} debug libmysql)
    target_link_libraries(${CONFIG_PROJECT_NAME} debug libcurl-d_imp)
    target_link_libraries(${CONFIG_PROJECT_NAME} optimized libmysql)
    target_link_libraries(${CONFIG_PROJECT_NAME} optimized libcurl_imp)
else()
    target_link_libraries(${CONFIG_PROJECT_NAME} debug libmysqlclient.so)
    target_link_libraries(${CONFIG_PROJECT_NAME} debug libcurl-d.so)
    target_link_libraries(${CONFIG_PROJECT_NAME} optimized libmysqlclient.so)
    target_link_libraries(${CONFIG_PROJECT_NAME} optimized libcurl.so)
endif()




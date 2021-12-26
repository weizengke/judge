cmake_minimum_required(VERSION 3.4.1)

project(judger VERSION 1.0.4 LANGUAGES C CXX)

set(TARGET judger)

message("MINGW=${MINGW}")
message("CYGWIN=${CYGWIN}")
message("WIN32=${WIN32}")
message("MSVC=${MSVC}")
message("CMAKE_CXX_COMPILER_ID=${CMAKE_CXX_COMPILER_ID}")
message("CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}")
message("CMAKE_SYSTEM=${CMAKE_SYSTEM}")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/release)

if (MINGW)
	add_definitions("-DMINGW")	
	set(EXTRA_LIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/MinGW/lib")
	set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/MinGW/libs)	
	set(COMPILE_DEFAULT_OPTION "-w -g -c -m32 -DWIN32 -std=gnu++0x -DGTEST_ENABLE")
	set(EXTRA_DEFAULT_LIBS -lwsock32 -lcurl -lwldap32 -lssl -lcrypto -lws2_32 mysql dbghelp psapi)
	set(LINK_STATIC_OPT -static)
elseif (CYGWIN)
	add_definitions("-DCYGWIN")
elseif (MSVC)
	add_definitions("-DMSVC")
	set(EXTRA_LIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/VSBuild/lib")
	set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/VSBuild/libs)
	set(COMPILE_DEFAULT_OPTION "/nologo /W0 /wd4127 /FD /c /EHsc /MD /Zi /DWIN32 /DHAVE_CONFIG_H /DPCRE_STATIC")
	set(EXTRA_DEFAULT_LIBS 
		User32.lib ws2_32.lib winmm.lib wldap32.lib psapi.lib crypt32.lib
		advapi32.lib shell32.lib dbghelp.Lib gdi32.lib mysqlclient.lib
		libcurl.lib libeay32.lib ssleay32.lib)
else()			
	if (ubuntu) #cmake -D ubuntu=1 ..
		set(EXTRA_LIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/ubuntu/lib")
		set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/ubuntu/libs)	
		set(EXTRA_DEFAULT_LIBS -lmysql -lcurl -ldl -lgtest -lgmock -lcrypt -lpthread)
	else() # suse
		set(EXTRA_LIBS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/suse/lib")
		set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/suse/libs)
		set(EXTRA_DEFAULT_LIBS -lmysql -lcurl -ldl -lgtest  -lssl -lcrypto -lgmock -lcrypt -lpthread)
	endif()
	set(COMPILE_DEFAULT_OPTION "-w -c -D_LINUX_ -std=gnu++11")
endif()

message("CMAKE_CXX_FLAGS_RELEASE=${CMAKE_CXX_FLAGS_RELEASE}")
message("CMAKE_C_FLAGS_RELEASE=${CMAKE_C_FLAGS_RELEASE}")

include_directories(
"${CMAKE_CURRENT_SOURCE_DIR}/code/include/"
"${CMAKE_CURRENT_SOURCE_DIR}/code/util/"
"${CMAKE_CURRENT_SOURCE_DIR}/code/third_party/bounds_checking_function/include/"
"${CMAKE_CURRENT_SOURCE_DIR}/code/third_party/"
"${CMAKE_CURRENT_SOURCE_DIR}/code/"
)

add_definitions(${COMPILE_DEFAULT_OPTION})
link_directories(${EXTRA_LIBS_DIR} ${LIBRARY_OUTPUT_PATH})
link_libraries(${EXTRA_DEFAULT_LIBS})

add_subdirectory(code)
add_executable(judger code code/main/main.cpp)

if (MSVC)
set_target_properties(judger PROPERTIES LINK_FLAGS /SUBSYSTEM:CONSOLE /NODEFAULTLIB:library /MACHINE:X86)
endif()

target_link_libraries(judger
	${LINK_STATIC_OPT}
	${EXTRA_LIBS}
	sysmng
	cli
	ftp
	telnet
	util
	judge
	ic
	aaa
	event
    ndp
	cjson
	pcre
	mytest
	securec
	)


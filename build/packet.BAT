g++.exe -o  bin/JungleOS -L"\lib"  obj/pdt_init.o obj/debug_center.o  obj/command_var.o obj/command_func.o obj/command_core.o obj/judge.o obj/judge_util.o obj/jungle_regex.o  -l wsock32 lib\libmysql.a lib\libmysql.dll lib\psapi.a lib\psapi.dll lib\pcre3.a lib\pcre3.dll lib\libcurl.a lib\libcurl.dll

pause


rem reimp **.lib
rem pexports psapi.dll > psapi.def
rem dlltool --input-def psapi.def --dllname psapi.dll --output-lib psapi.a -k


rem pexports pcre3.dll > pcre3.def
rem dlltool --input-def pcre3.def --dllname pcre3.dll --output-lib pcre3.a -k

rem libcurl.dll zlib1.dll for libcurl
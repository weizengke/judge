mingw32-make

g++.exe -o  bin/JungleOS -L"\lib" obj/pdt_init.o obj/debug_center.o  obj/command_var.o obj/command_func.o obj/command_core.o obj/command_adp.o obj/command_io.o obj/judge.o obj/judge_util.o obj/judge_var.o obj/judge_hdu.o obj/judge_io.o obj/judge_sql.o obj/jungle_regex.o -l wsock32 lib\libmysql.a lib\libmysql.dll lib\psapi.a lib\psapi.dll lib\pcre3.a lib\pcre3.dll lib\libcurl.a lib\libcurl.dll

cd bin
JungleOS.exe

pause
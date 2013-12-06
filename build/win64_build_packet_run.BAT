mingw32-make -f makefile64

g++.exe -o  bin64/JungleOS -L"\lib64" -static obj/pdt_init.o obj/debug_center.o  obj/command_var.o obj/command_func.o obj/command_core.o obj/command_adp.o obj/command_io.o obj/judge.o obj/judge_util.o obj/judge_var.o obj/judge_hdu.o obj/judge_io.o obj/judge_sql.o obj/jungle_regex.o -l wsock32 lib64\libmysql.a lib64\libmysql.dll lib64\psapi.a lib64\psapi.dll lib64\pcre3.a lib64\pcre3.dll lib64\libcurl.a lib64\libcurl.dll

cd bin64
JungleOS.exe

pause
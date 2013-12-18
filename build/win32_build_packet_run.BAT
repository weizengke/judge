mingw32-make -f makefile32

g++.exe -static -o  bin32/JungleOS -L"\lib32" obj/pdt_init.o obj/debug_center.o  obj/command_var.o obj/command_func.o obj/command_core.o obj/command_adp.o obj/command_io.o obj/judge.o obj/judge_util.o obj/judge_var.o obj/judge_hdu.o obj/judge_io.o obj/judge_sql.o obj/jungle_regex.o -l wsock32 lib32\libmysql.a lib32\libmysql.dll lib32\psapi.a lib32\psapi.dll lib32\pcre.a lib32\pcre.dll lib32\curllib.a lib32\curllib.dll

cd bin32
JungleOS.exe

pause
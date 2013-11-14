x86_64-w64-mingw32-g++.exe -o  bin/JungleOS -L"\lib"  obj/pdt_init.o obj/debug_center.o  obj/cmd.o obj/judge.o obj/judge_util.o  -l wsock32 lib\libmysql.a lib\libmysql.dll lib\psapi.a lib\psapi.dll lib\psapi.lib  
pause 


rem pexports psapi.dll > psapi.def
rem dlltool --input-def psapi.def --dllname psapi.dll --output-lib psapi.a -k
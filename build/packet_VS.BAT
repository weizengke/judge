link *.obj  /LIBPATH:"/lib" /out:bin/JungleOS  lib\psapi.lib
pause

rem pexports psapi.dll > psapi.def
rem dlltool --input-def psapi.def --dllname psapi.dll --output-lib psapi.a -k


rem pexports regex2.dll > regex2.def
rem dlltool --input-def regex2.def --dllname regex2.dll --output-lib regex.a -k
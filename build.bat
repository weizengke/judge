
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\VsDevCmd.bat"

if not exist vsbuild (
	md vsbuild
)

cd vsbuild

cmake -G "NMake Makefiles" ..
cmake --build .

pause
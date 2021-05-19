
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\VsDevCmd.bat"

xcopy /y CMakeLists_vs2017.txt CMakeLists.txt

if not exist vsbuild (
	md vsbuild
)

cd vsbuild

cmake -G "NMake Makefiles" ..
cmake --build .

pause
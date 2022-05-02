@echo off
:loop
timeout 30 /nobreak
tasklist | find "judger" >nul || start "" "startup.bat"
goto :loop
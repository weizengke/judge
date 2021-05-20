@echo off
:loop
timeout 30 /nobreak
taskkill /im javae.exe /f
tasklist | find "judger" >nul || start "" "startup.bat"
goto :loop
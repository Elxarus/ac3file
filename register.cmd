@echo off
echo ------------------------
echo Register %1
set filter=%1\ac3file.ax
if not exist "%filter%" set filter=%1\ac3file64.ax

regsvr32 /s /u "%filter%"
regsvr32 /s "%filter%"

if errorlevel 1 goto err
goto end

:err
echo Error! Cannot register %filter%.
fail 2>nul

:end

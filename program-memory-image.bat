@ECHO OFF

IF %1.==. GOTO Help

IF %1==1052 GOTO Do
IF %1==1062 GOTO Do

:Help
echo.
echo Program writes memory-image.sb file from the local directory to flash memory of the attached i.MX RT
echo To edit  USB configuration, edit the file program-flash-usb.bat
echo.
echo Usage: program-memory-image.bat devicetype
echo        devicetype  1052 or 1062
echo.
EXIT /B 2

:Do

set BATCH_DIR=%~dp0

call %BATCH_DIR%\program-flash-usb.bat %1 .\memory-image.sb

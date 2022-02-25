@ECHO OFF

IF %1.==. GOTO Help
IF %2.==. GOTO Help

IF %1==1052 GOTO Do
IF %1==1062 GOTO Do

:Help
echo Script programs device flash memory over USB using the input secure binary file
echo.
echo Usage: program-flash-serial.bat devicetype secure-binary-file
echo        devicetype  1052 or 1062
echo.
EXIT /B 2

:Do
@set COMPAR=-u

set BATCH_DIR=%~dp0
set TOOLS_DIR=%BATCH_DIR%..\..\third_party\nxp_tools_win
set SB_FILE=%~f2

IF NOT exist %SB_FILE% (
   echo.
   echo File %SB_FILE% does not exists!
   echo. 
   GOTO :Failure
)

@call %BATCH_DIR%\load-flashloader-usb.bat %1

if not "%ERRORLEVEL%" == "0" (
   echo.
   echo Failed to load flashloader
   echo.
   GOTO :Failure
)

%TOOLS_DIR%\blhost %COMPAR% -t 300000 -- receive-sb-file %SB_FILE% || GOTO Failure
%TOOLS_DIR%\blhost %COMPAR% -- reset || GOTO Failure

EXIT /B 0

:Failure
EXIT /B 1

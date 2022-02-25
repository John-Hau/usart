@ECHO OFF

IF %1.==. GOTO Help

IF %1==1052 GOTO Do
IF %1==1062 GOTO Do

:Help
echo Program burns efuses using the secure binary file burn-efuses.sb
echo burn-efuses.sb is created by the create-efuse-burning-sb.bat script.
echo.
echo Usage: burn-efuses.bat devicetype
echo        devicetype  1052 or 1062
echo.
EXIT /B 2

:Do

IF NOT exist burn-efuses.sb GOTO Help

@set COMPAR=-u

set BATCH_DIR=%~dp0
set TOOLS_DIR=%BATCH_DIR%..\..\third_party\nxp_tools_win

IF %1==1052 (
   set VIDPID=0x1fc9,0x0130
   set FLASHLOADER=%BATCH_DIR%\files\mimxrt1050_ivt_flashloader_qa_signed.bin
)

IF %1==1062 (
   set VIDPID=0x1fc9,0x0135
   set FLASHLOADER=%BATCH_DIR%\files\mimxrt1060_ivt_flashloader_qa_signed.bin
)

%TOOLS_DIR%\sdphost %COMPAR% %VIDPID%  -- write-file 0x20000000 %FLASHLOADER%
%TOOLS_DIR%\sdphost %COMPAR% %VIDPID%  -- jump-address 0x20000400

TIMEOUT 1

%TOOLS_DIR%\blhost %COMPAR% -- get-property 1
%TOOLS_DIR%\blhost %COMPAR% -t 10000 -- receive-sb-file burn-efuses.sb

EXIT /B 0


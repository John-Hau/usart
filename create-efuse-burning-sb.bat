@ECHO OFF

set BATCH_DIR=%~dp0
set TOOLS_DIR=%BATCH_DIR%..\..\third_party\nxp_tools_win


echo Creating SB file for eFuse burning

%TOOLS_DIR%\elftosb.exe -f kinetis -V -c "%BATCH_DIR%bd-files\burn-efuses-qa-keys.bd" -o burn-efuses.sb 

echo.

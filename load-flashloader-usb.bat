@IF %1.==. GOTO Help
@GOTO Do

:Help
@echo Script loads flashloader, starts it and waits until it starts responding to blhost commands.
@echo.
@echo Usage: load-flashloader.bat ^<1062^|1052^>
@echo.
@EXIT /B 2

:Do
@set COMPAR=-u

@if "%~1" == "1062" (set VIDPID=0x1fc9,0x0135 && set FLASHLOADER=mimxrt1060_ivt_flashloader_qa_signed.bin && goto Sdphost)
@if "%~1" == "1052" (set VIDPID=0x1fc9,0x0130 && set FLASHLOADER=mimxrt1050_ivt_flashloader_qa_signed.bin && goto Sdphost)
@echo Missing or unknown board type '%1'
@GOTO Help

:Sdphost
@set BATCH_DIR=%~dp0
@set TOOLS_DIR=%BATCH_DIR%..\..\third_party\nxp_tools_win

@rem Try the first sdphost command several times to give Windows some time to setup the USB device
@set SDPHOST_RETRY_COUNT=10
:SdphostRetry
@%TOOLS_DIR%\sdphost %COMPAR% %VIDPID% -- write-file 0x20000000 %BATCH_DIR%\files\%FLASHLOADER%
@if not "%ERRORLEVEL%" == "0" (
   IF %SDPHOST_RETRY_COUNT% leq 1 ( echo. & GOTO :Failure )
   set /a SDPHOST_RETRY_COUNT -= 1
   C:\Windows\System32\waitfor.exe SomethingThatIsNeverHappening /t 1  2>NUL
   echo.
   GOTO :SdphostRetry
)

@%TOOLS_DIR%\sdphost %COMPAR% %VIDPID% -- jump-address 0x20000400 || GOTO Failure

@rem Try a blhost command several times to give Windows some time to setup the USB device
@set BLHOST_RETRY_COUNT=10
:BlhostRetry
@%TOOLS_DIR%\blhost %COMPAR% -- get-property 1
@if not "%ERRORLEVEL%" == "0" (
   IF %BLHOST_RETRY_COUNT% leq 1 ( echo. & GOTO :Failure )
   set /a BLHOST_RETRY_COUNT -= 1
   C:\Windows\System32\waitfor.exe SomethingThatIsNeverHappening /t 1  2>NUL
   echo.
   GOTO :BlhostRetry
)

@EXIT /B 0

:Failure
@EXIT /B 1

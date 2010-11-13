
IF EXIST "%HOMEPATH%\Application Data\Wireshark\plugins" GOTO COPYDLL
mkdir "%HOMEPATH%\Application Data\Wireshark\plugins"
:COPYDLL

copy /B /Y fast.dll "%HOMEPATH%\Application Data\Wireshark\plugins\fast.dll"

pause


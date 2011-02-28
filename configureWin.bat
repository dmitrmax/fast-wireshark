REM Compile/install libxml2
cd c:\compile\libxml2-2.7.6\win32
cscript configure.js compiler=msvc ^
prefix=c:\dev include=c:\dev\include lib=c:\dev\lib ^
http=no ftp=no
nmake /f Makefile.msvc
nmake /f Makefile.msvc install

REM CheckFor/Make wireshark plugin folder
IF EXIST "%HOMEPATH%\Application Data\Wireshark\plugins" GOTO BUILD
mkdir "%HOMEPATH%\Application Data\Wireshark\plugins"
:BUILD

REM Build the plugin
cd c:\fast-wireshark
build.bat


REM Compile/install libxml2
cd c:\compile\libxml\win32
cscript configure.js compiler=msvc ^
prefix=c:\dev include=c:\dev\include lib=c:\dev\lib ^
http=no ftp=no
nmake /f Makefile.msvc
nmake /f Makefile.msvc install

REM Build the plugin
cd c:\fast-wireshark
cmake -G "NMake Makefiles"
nmak
nmake install
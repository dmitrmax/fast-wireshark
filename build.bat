REM Setup paths
call paths.bat

REM Build the plugin
cmake -G "NMake Makefiles"
nmake
call install.bat

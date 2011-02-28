REM Setup paths
paths.bat

REM Build the plugin
cmake -G "NMake Makefiles"
nmake
install.bat

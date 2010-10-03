
set tmp_path=%PATH%
REM call "%PROGRAMFILES%\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"
set path=C:\dev\bin;%PATH%
set PKG_CONFIG_PATH=C:\dev\lib\pkgconfig
cmake -G "NMake Makefiles"
set path=%tmp_path%


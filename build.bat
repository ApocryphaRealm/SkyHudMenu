@echo off
setlocal
REM Build the plugin. Run configure.bat first if the build directory does not exist yet.

call "%~dp0find-msvc.bat"
if errorlevel 1 exit /b 1

cd /d "%~dp0"
cmake --build build/relwithdebinfo-se-only

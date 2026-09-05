@echo off
setlocal
REM Configure the CMake build. This is the step that makes vcpkg build CommonLibSSE-NG and
REM friends, so the first run takes a while.
REM
REM Requires VCPKG_ROOT to point at a vcpkg checkout. Visual Studio, and the CMake and Ninja
REM that ship with it, are located by find-msvc.bat.

call "%~dp0find-msvc.bat"
if errorlevel 1 exit /b 1

if "%VCPKG_ROOT%"=="" (
	echo VCPKG_ROOT is not set. Point it at your vcpkg checkout and try again.
	exit /b 1
)

cd /d "%~dp0"
cmake --preset build-relwithdebinfo-se-only

::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
::
:: Copyright (c) 2019 Livox. All rights reserved.
::
:: Permission is hereby granted, free of charge, to any person obtaining a copy
:: of this software and associated documentation files (the "Software"), to deal
:: in the Software without restriction, including without limitation the rights
:: to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
:: copies of the Software, and to permit persons to whom the Software is
:: furnished to do so, subject to the following conditions:
::
:: The above copyright notice and this permission notice shall be included in
:: all copies or substantial portions of the Software.
::
:: THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
:: IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
:: FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
:: AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
:: LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
:: OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
:: SOFTWARE.
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

@echo off


if "%1" == "" (
	set PLATFORM=x86
) else if "%1" == "x86" (
	set PLATFORM=x86 
) else if "%1" == "amd64" (
	set PLATFORM=amd64
) else (
	goto :usage
)

if defined VS140COMNTOOLS (
	call "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat" %PLATFORM%
)

if defined VS150COMNTOOLS (
	call "%VS150COMNTOOLS%..\..\VC\Auxiliary\Build\vcvarsall.bat" %PLATFORM%
) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" (
	call "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" %PLATFORM%
) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvarsall.bat" (
	call "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvarsall.bat" %PLATFORM%
) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" (
	call "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" %PLATFORM%
) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" (
	call "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" %PLATFORM%
) 

if defined VS160COMNTOOLS (
	call "%VS160COMNTOOLS%..\..\VC\Auxiliary\Build\vcvarsall.bat" %PLATFORM%
) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" (
	call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" %PLATFORM%
) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat" (
	call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvarsall.bat" %PLATFORM%
) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" (
	call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" %PLATFORM%
) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" (
	call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" %PLATFORM%
) 

echo start to build.

pushd %~dp0apr-1.6.5

::creat temp build folder
if not exist temp (
	mkdir temp
)

pushd temp

cmake .. -G "NMake Makefiles" ^
		-DCMAKE_INSTALL_PREFIX=%~dp0 ^
		-DAPR_INSTALL_PRIVATE_H=OFF ^
		-DAPR_HAVE_IPV6=OFF ^
		-DINSTALL_PDB=OFF ^
		-DAPR_BUILD_TESTAPR=OFF ^
		-DTEST_STATIC_LIBS=OFF ^
		-DCMAKE_BUILD_TYPE=Release
::if %ERRORLEVEL% neq 0 (echo "error: cmake generate error") && exit /b 1
nmake
nmake install

popd
rd/s/q temp
popd
goto :eof

:usage
echo Error in script usage. The Correct usage is:
echo %0 [option]
echo where [option] is: x86^| amd64
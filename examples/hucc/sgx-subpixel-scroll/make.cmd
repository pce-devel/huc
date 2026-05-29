@rem ************************************************************************
@rem ************************************************************************
@rem
@rem make.cmd
@rem
@rem Build a project with a native Windows version of Linux and macOS "make".
@rem
@rem Copyright John Brandwood 2025.
@rem
@rem Distributed under the Boost Software License, Version 1.0.
@rem (See accompanying file LICENSE_1_0.txt or copy at
@rem  http://www.boost.org/LICENSE_1_0.txt)
@rem
@rem ************************************************************************
@rem ************************************************************************
@rem
@rem Put this in your project's directory on Windows to automatically set the
@rem PATH and PCE_INCLUDE environment variables if your project is located in
@rem a directory within the main HuC folder tree.
@rem
@rem Then mingw32-make.exe is run to build the project using its Makefile.
@rem
@rem You can run this from a Windows "Command Prompt", or by navigating to it
@rem in Windows Explorer and then double-clicking on the file.
@rem
@rem ************************************************************************
@rem ************************************************************************

@echo off

setlocal

call :findexe hucc.exe
if not [%EXEFILE%] == [""] goto :gotpath 

cd /d "%~dp0"
set rootdir="%~d0\"
:search
if not exist "%CD%\bin" goto :next
if not exist "%CD%\bin\hucc.exe" goto :next
set PATH=%CD%\bin;%PATH%
set PCE_INCLUDE=%CD%\include\hucc
goto :gotpath
:next
if not ["%CD%"] == [%rootdir%] (
  cd ..
  goto :search
)
echo.
echo Unable to locate hucc.exe, please set up your PATH and PCE_INCLUDE
echo environment variables!
if /i "%comspec% /c %~0 " equ "%cmdcmdline:"=%" (
  echo.
  pause
)
exit /b 1

:findexe
set EXEFILE="%~$PATH:1"
goto :eof

:gotpath
cd /d "%~dp0"
mingw32-make.exe %*

rem Pause if this was run by doubleclicking on the file in Explorer.
if /i "%comspec% /c %~0 " equ "%cmdcmdline:"=%" (
  echo.
  pause
)

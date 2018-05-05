@ECHO off

REM MSVC build bat file
REM run with vscmd: cmd /k "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\VsDevCmd.bat"

set target=wavbuff_w32.exe
set builddir=winbuild

IF not [%1]==[] (GOTO args)

mkdir %builddir%

SET portaudio_lib=D:\Libraries\portaudio\build\msvc
SET portaudio_include=D:\Libraries\portaudio\include

SET libdirs=/LIBPATH:%portaudio_lib% 
SET libs=portaudio_x86.lib 

SET includes=/I%portaudio_include% /Ilib

SET files=main.cpp lib\pa.cpp lib\pa_ringbuffer.c lib\wavutil.c lib\rb.c

SET opts=/Fe%target% /Fo%builddir%\
rem SET opts=/Fe%builddir%/%target% /Fo%builddir%/

cl %opts% %includes% %files% /link %libdirs% %libs%

GOTO end

:args
IF [%1]==[clean] (del %builddir%\* del %target%)
IF [%1]==[run] (%target%)

:end

@ECHO OFF
timeout 1
cd /d "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build"
call vcvarsall.bat x86
cd /d D:\3s\KPKPO\TEA-2025\Debug
ml /c /coff /Zi input.txt.asm
link /OPT:NOREF /DEBUG /SUBSYSTEM:CONSOLE ^
    /NODEFAULTLIB:libcmt.lib ^
    /NODEFAULTLIB:libvcruntime.lib ^
    /NODEFAULTLIB:libucrt.lib ^
    input.txt.obj ^
    BVS-2024Lib.lib ^
    libcmtd.lib ^
    libvcruntimed.lib ^
    ucrtd.lib ^
    legacy_stdio_definitions.lib ^
    kernel32.lib 
call input.txt.exe
timeout 5
pause
exit
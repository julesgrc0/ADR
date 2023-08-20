@echo off
@REM && upx --best main.exe

if exist gADR.exe (
    del gADR.exe
)
garble build -ldflags "-s -w" main && rename main.exe gADR.exe
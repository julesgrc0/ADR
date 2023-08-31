@echo off
@REM && upx --best main.exe

if exist goloader.exe (
    del goloader.exe
)
garble build -ldflags "-s -w" main && rename main.exe goloader.exe
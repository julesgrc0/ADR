#pragma once
#include "../stdafx.h"
#include "../utils.h"

class WebhookEmbed {
public:
	WebhookEmbed();

	operator json ();

	void SetTitle(const std::string&);
	void SetDescription(const std::string&);
	void SetURL(const std::string&);
	void SetColor(int);

	void SetTimestamp();

	void SetAuthor(const std::string& name, const std::string& url = std::string(), const std::string& icon = std::string());
	void SetFooter(const std::string& text, const std::string& icon = std::string());
	void SetImage(const std::string& url, int w = -1, int h = -1);
	void SetThumbnail(const std::string& url, int w = -1, int h = -1);
	void SetVideo(const std::string& url, int w = -1, int h = -1);

	std::unordered_map<std::string, std::string> fields;
private:
	json embed;
};

class Webhook
{
public:
	Webhook(std::string token, long long id);
	~Webhook();

	void AddContentLine(const std::string&, const std::string&);
	void AddContent(const std::string&);

	std::string username;
	std::string avatar_url;

	std::vector<WebhookEmbed> embeds;
	std::vector<std::string> files;
	std::vector<std::string> attachments;

	bool Send();
private:
	std::string url;
	std::stringstream content;
};

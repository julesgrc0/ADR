#include "webhook.h"

Webhook::Webhook(std::string token,long long id)
{
	this->url = "https://discord.com//api/webhooks/"+ std::to_string(id) + "/" + token; 
}

Webhook::~Webhook()
{
}

void Webhook::AddContentLine(const std::string& key, const std::string& val)
{
	this->content << key << ':' << ' ' << val << '\n';
}

void Webhook::AddContent(const std::string& raw)
{
	this->content << raw;
}

bool Webhook::Send()
{
	CURL* curl = curl_easy_init();
	if (!curl) return false;

	struct curl_httppost* formpost = nullptr;
	struct curl_httppost* lastptr = nullptr;

	json payload;
	
	if (this->username.length() > 0) payload["username"] = this->username;
	if (this->avatar_url.length() > 0) payload["avatar_url"] = this->avatar_url;

	std::string contentstr = this->content.str();
	if (contentstr.length() > 0) payload["content"] = contentstr;

	payload["tts"] = false;
	for (const json& embed : this->embeds)
	{
		payload["embeds"].emplace_back(embed);
	}
	payload["attachments"] = this->attachments;
	std::string jsonPayload = payload.dump();

	curl_formadd(&formpost, &lastptr,
		CURLFORM_COPYNAME, "payload_json",
		CURLFORM_COPYCONTENTS, jsonPayload.c_str(),
		CURLFORM_END);

	for (const std::string& file : this->files)
	{
		curl_formadd(&formpost, &lastptr,
			CURLFORM_COPYNAME, HPathGetName(file).c_str(),
			CURLFORM_FILE, file.c_str(),
			CURLFORM_END);
	}

	curl_easy_setopt(curl, CURLOPT_URL, this->url.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

	CURLcode res = curl_easy_perform(curl);

	curl_formfree(formpost);
	curl_easy_cleanup(curl);
	return true;
}

WebhookEmbed::WebhookEmbed()
{

}

WebhookEmbed::operator json()
{
	this->embed["fields"].clear();
	this->embed["fields"] = std::vector<json>();

	for (auto fieldObj : this->fields)
	{
		json field;
		field["name"] = fieldObj.first;
		field["value"] = fieldObj.second;
		field["inline"] = true;

		this->embed["fields"].emplace_back(field);
	}

	return this->embed;
}

void WebhookEmbed::SetTitle(const std::string& title)
{
	this->embed["title"] = title;
}

void WebhookEmbed::SetDescription(const std::string& desc)
{
	this->embed["description"] = desc;
}

void WebhookEmbed::SetURL(const std::string& url)
{
	this->embed["url"] = url;
}

void WebhookEmbed::SetTimestamp()
{
	auto now = std::chrono::system_clock::now();
	std::time_t time = std::chrono::system_clock::to_time_t(now);

	char buffer[30];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", std::localtime(&time));

	auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

	std::stringstream st;
	st << buffer << "." << std::setw(3) << std::setfill('0') << milliseconds.count() << "Z";
	this->embed["timestamp"] = st.str();
}

void WebhookEmbed::SetColor(int color)
{
	this->embed["color"] = color;
}


void WebhookEmbed::SetAuthor(const std::string& name, const std::string& url, const std::string& icon)
{
	if (name.length() > 0) this->embed["author"]["name"] = name;
	if (url.length() > 0) this->embed["author"]["url"] = url;
	if (icon.length() > 0) this->embed["author"]["icon_url"] = icon;
}

void WebhookEmbed::SetFooter(const std::string& text, const std::string& icon)
{
	if (text.length() > 0) this->embed["footer"]["text"] = text;
	if (icon.length() > 0) this->embed["footer"]["icon_url"] = icon;
}

void WebhookEmbed::SetImage(const std::string& url, int w, int h)
{
	this->embed["image"]["url"] = url;
	if (w != -1) this->embed["image"]["width"] = w;
	if (h != -1) this->embed["image"]["height"] = h;
}

void WebhookEmbed::SetThumbnail(const std::string& url, int w, int h)
{
	this->embed["thumbnail"]["url"] = url;
	if (w != -1) this->embed["thumbnail"]["width"] = w;
	if (h != -1) this->embed["thumbnail"]["height"] = h;
}

void WebhookEmbed::SetVideo(const std::string& url, int w, int h)
{
	this->embed["video"]["url"] = url;
	if(w != -1) this->embed["video"]["width"] = w;
	if(h != -1) this->embed["video"]["height"] = h;
}

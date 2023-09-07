#pragma once

#include "squirrel/squirrel.h"

//-----------------------------------------------------------------------------
//
template <ScriptContext context>
class CScriptJson
{
  public:
	static void DecodeJson(HSquirrelVM* sqvm, nlohmann::json& jsObj);

	static void EncodeJsonTable(SQTable* pTable, nlohmann::json& jsObj);
	static void EncodeJsonArray(SQArray* pArray, nlohmann::json& jsObj);

  private:
	static void DecodeTable(HSquirrelVM* sqvm, nlohmann::json& jsObj);
	static void DecodeArray(HSquirrelVM* sqvm, nlohmann::json& jsObj);
};

//-----------------------------------------------------------------------------
//
class CScriptHttp
{
  public:
	enum Type
	{
		HTTP_GET = 0,
		HTTP_POST = 1,
		HTTP_HEAD = 2,
		HTTP_PUT = 3,
		HTTP_DELETE = 4,
		HTTP_PATCH = 5,
		HTTP_OPTIONS = 6,
	};

	struct HttpRequest_t
	{
		// Method used for this http request.
		Type eMethod;

		// Base URL of this http request.
		std::string svBaseUrl;

		// Headers used for this http request. Some may get overridden or ignored.
		std::unordered_map<std::string, std::vector<std::string>> mpHeaders;

		// Query parameters for this http request.
		std::unordered_map<std::string, std::vector<std::string>> mpQueryParameters;

		// The content type of this http request. Defaults to text/plain & UTF-8 charset.
		std::string svContentType = "text/plain; charset=utf-8";

		// The body of this http request. If set, will override queryParameters.
		std::string svBody;

		// The timeout for the http request, in seconds. Must be between 1 and 60.
		int nTimeout;

		// If set, the override to use for the User-Agent header.
		std::string svUserAgent;
	};

	CScriptHttp();

	inline bool IsHttpDisabled() const
	{
		return m_bIsHttpDisabled;
	}

	inline bool IsLocalHttpAllowed() const
	{
		return m_bIsHttpDisabled;
	}

	inline std::string ToString(Type method)
	{
		switch (method)
		{
		case HTTP_GET:
			return "GET";
		case HTTP_POST:
			return "POST";
		case HTTP_HEAD:
			return "HEAD";
		case HTTP_PUT:
			return "PUT";
		case HTTP_DELETE:
			return "DELETE";
		case HTTP_PATCH:
			return "PATCH";
		case HTTP_OPTIONS:
			return "OPTIONS";
		default:
			return "INVALID";
		}
	}

	inline bool UsesCurlPostOptions(Type method)
	{
		switch (method)
		{
		case HTTP_POST:
		case HTTP_PUT:
		case HTTP_DELETE:
		case HTTP_PATCH:
			return true;
		default:
			return false;
		}
	}

	inline bool CanHaveQueryParameters(Type method)
	{
		return method == HTTP_GET || UsesCurlPostOptions(method);
	}

	bool IsHttpDestinationHostAllowed(const std::string& host, std::string& outHostname, std::string& outAddress, std::string& outPort);

	template <ScriptContext context>
	int MakeHttpRequest(const HttpRequest_t& requestParameters);

  private:
	int m_iLastRequestHandle = 0;

	bool m_bIsHttpDisabled = false;
	bool m_bIsLocalHttpAllowed = false;
};

inline CScriptHttp* g_pScriptHttp = nullptr;

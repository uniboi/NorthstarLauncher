#include "pch.h"
#include "httpclient.h"

#include "squirrel.h"

void SetCommonHttpClientOptions(CURL* curl)
{
	curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
}

size_t CurlWriteToStringBufferCallbackLocal(char* contents, size_t size, size_t nmemb, void* userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

char* MakePost(const char* url, const char* body)
{
	CURL* curl = curl_easy_init();

	std::string readBuffer;

	curl_easy_setopt(
		curl,
		CURLOPT_URL,
		url
		);
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteToStringBufferCallbackLocal);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
	SetCommonHttpClientOptions(curl);

	CURLcode res = curl_easy_perform(curl);

	if (res != CURLE_OK)
		fprintf(
			stderr, "curl_easy_perform() failed: %s\n",
			curl_easy_strerror(res)
			);

	curl_easy_cleanup(curl);



	char* writable = new char[readBuffer.size() + 1];
	std::copy(readBuffer.begin(), readBuffer.end(), writable);
	writable[readBuffer.size()] = '\0';

	return writable;
}


static SQRESULT SQ_SendPostRequest(void* sqvm)
{
	const char* url = ServerSq_getstring(sqvm, 1);
	const char* body = ServerSq_getstring(sqvm, 2);

	char* result = MakePost(url, body);

	ServerSq_pushstring(sqvm, result, -1);

	return SQRESULT_NOTNULL;
}

void InitialiseHttpClient(HMODULE baseAddress)
{
	g_ServerSquirrelManager->AddFuncRegistration("string", "NSMakePostRequest", "string url, string body", "", SQ_SendPostRequest);
}

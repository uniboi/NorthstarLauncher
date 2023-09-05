#pragma once

struct CURLParms
{
	CURLParms() : pWriteFunc(nullptr), nTimeout(30), bVerifyHost(false), bVerifyPeer(false) {}

	void* pWriteFunc;
	int nTimeout;
	bool bVerifyHost;
	bool bVerifyPeer;
};

struct CURLMime
{
	CURLMime() : svData(""), svFileName(""), svName(""), svType("") {}

	std::string svData;
	std::string svFileName;
	std::string svName;
	std::string svType;
};

size_t CURLWriteStringCallback(char* contents, size_t size, size_t nmemb, std::string* userp);

CURL* CURLInitRequest(const char* pszUrl, const char* pszRequest, std::string& svResponse, CURLParms& parms, CURLMime *pMime = nullptr);
CURLcode CURLSubmitRequest(CURL* curl);
void CURLCleanup(CURL* curl);
long CURLGetResponse(CURL* curl);

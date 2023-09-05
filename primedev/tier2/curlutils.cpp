#include "tier2/curlutils.h"

size_t CURLWriteStringCallback(char* contents, size_t size, size_t nmemb, std::string* userp)
{
	userp->append(contents, size * nmemb);
	return size * nmemb;
}

CURL* CURLInitRequest(const char* pszUrl, const char* pszRequest, std::string& svResponse, CURLParms& parms, CURLMime* pMime)
{
	CURL* curl = curl_easy_init();

	// Default
	curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, &NORTHSTAR_USERAGENT);

	curl_easy_setopt(curl, CURLOPT_URL, pszUrl);

	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, pszRequest);

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, parms.pWriteFunc);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, parms.nTimeout);

	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, parms.bVerifyHost);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, parms.bVerifyPeer);

	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &svResponse);

	if (pMime)
	{
		curl_mime* mime = curl_mime_init(curl);
		curl_mimepart* part = curl_mime_addpart(mime);

		curl_mime_data(part, pMime->svData.c_str(), pMime->svData.size());
		curl_mime_name(part, pMime->svName.c_str());
		curl_mime_filename(part, pMime->svFileName.c_str());
		curl_mime_type(part, pMime->svType.c_str());

		curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
	}

	return curl;
}

CURLcode CURLSubmitRequest(CURL* curl)
{
	return curl_easy_perform(curl);
}

void CURLCleanup(CURL* curl)
{
	curl_easy_cleanup(curl);
}


long CURLGetResponse(CURL* curl)
{
	long nResponse;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &nResponse);
	return nResponse;
}

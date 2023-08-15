#include "bcrypt.h"

#include <bcrypt.h>

BCRYPT_ALG_HANDLE HMACSHA256;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool InitHMACSHA256()
{
	NTSTATUS status;
	DWORD hashLength = 0;
	ULONG hashLengthSz = 0;

	if ((status = BCryptOpenAlgorithmProvider(&HMACSHA256, BCRYPT_SHA256_ALGORITHM, NULL, BCRYPT_ALG_HANDLE_HMAC_FLAG)))
	{
		Error(eLog::NS, NO_ERROR, "failed to initialize HMAC-SHA256: BCryptOpenAlgorithmProvider: error 0x%x\n", (ULONG)status);
		return false;
	}

	if ((status = BCryptGetProperty(HMACSHA256, BCRYPT_HASH_LENGTH, (PUCHAR)&hashLength, sizeof(hashLength), &hashLengthSz, 0)))
	{
		Error(eLog::NS, NO_ERROR, "failed to initialize HMAC-SHA256: BCryptGetProperty(BCRYPT_HASH_LENGTH): error 0x%x\n", (ULONG)status);
		return false;
	}

	if (hashLength != HMACSHA256_LEN)
	{
		Error(
			eLog::NS,
			NO_ERROR,
			"failed to initialize HMAC-SHA256: BCryptGetProperty(BCRYPT_HASH_LENGTH): unexpected value %i\n",
			hashLength);
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: compare the HMAC-SHA256(data, key) against sig
//          (note: all strings are treated as raw binary data)
//-----------------------------------------------------------------------------
bool VerifyHMACSHA256(std::string key, std::string sig, std::string data)
{
	uint8_t invalid = 1;
	char hash[HMACSHA256_LEN];

	NTSTATUS status;
	BCRYPT_HASH_HANDLE h = NULL;

	if ((status = BCryptCreateHash(HMACSHA256, &h, NULL, 0, (PUCHAR)key.c_str(), (ULONG)key.length(), 0)))
	{
		Error(eLog::NS, NO_ERROR, "failed to verify HMAC-SHA256: BCryptCreateHash: error 0x%x\n", (ULONG)status);
		goto cleanup;
	}

	if ((status = BCryptHashData(h, (PUCHAR)data.c_str(), (ULONG)data.length(), 0)))
	{
		Error(eLog::NS, NO_ERROR, "failed to verify HMAC-SHA256: BCryptHashData: error 0x%x\n", (ULONG)status);
		goto cleanup;
	}

	if ((status = BCryptFinishHash(h, (PUCHAR)&hash, (ULONG)sizeof(hash), 0)))
	{
		Error(eLog::NS, NO_ERROR, "failed to verify HMAC-SHA256: BCryptFinishHash: error 0x%x\n", (ULONG)status);
		goto cleanup;
	}

	// constant-time compare
	if (sig.length() == sizeof(hash))
	{
		invalid = 0;
		for (size_t i = 0; i < sizeof(hash); i++)
			invalid |= (uint8_t)(sig[i]) ^ (uint8_t)(hash[i]);
	}

cleanup:
	if (h)
		BCryptDestroyHash(h);
	return !invalid;
}

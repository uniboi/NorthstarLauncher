#pragma once

constexpr size_t HMACSHA256_LEN = 256 / 8;

bool InitHMACSHA256();
bool VerifyHMACSHA256(std::string key, std::string sig, std::string data);

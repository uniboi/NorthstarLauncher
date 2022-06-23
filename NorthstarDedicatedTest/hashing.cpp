#include "pch.h"
#include "hashing.h"

#include "squirrel.h"

#include <iostream>
#include <sstream>
#include <bitset>
#include <vector>
#include <iomanip>
#include <cstring>

/*
	SHA256 Hashing Function(s) by hak8or (https://github.com/hak8or/ | https://hak8or.com/)
	Original code with more comments is available here: https://gist.github.com/hak8or/8794351

	I've changed `compute_hash` because the function didn't return the same hash every time it got executed (which is kind of a core feature
   of hashing algorithms)

	Parts of the (original) code follow the specifications of this doc:
   https://csrc.nist.gov/csrc/media/publications/fips/180/4/final/documents/fips180-4-draft-aug2014.pdf Read it if you have no idea what's
   going on.
*/
class SHA256
{
#define ROTRIGHT(word, bits) (((word) >> (bits)) | ((word) << (32 - (bits))))
#define SSIG0(x) (ROTRIGHT(x, 7) ^ ROTRIGHT(x, 18) ^ ((x) >> 3))
#define SSIG1(x) (ROTRIGHT(x, 17) ^ ROTRIGHT(x, 19) ^ ((x) >> 10))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))

#define EP0(x) (ROTRIGHT(x, 2) ^ ROTRIGHT(x, 13) ^ ROTRIGHT(x, 22))
#define EP1(x) (ROTRIGHT(x, 6) ^ ROTRIGHT(x, 11) ^ ROTRIGHT(x, 25))

  private:
	/*
		Resize the blocks from 64 8 bit sections to 16 32 bit sections.
		Params:
		std::vector<unsigned long> input : Vector of individual 8 bit ascii values
		Returns:
		std::vector<unsigned long> : Vector of 32 bit words which are combined ascii values.
	*/
	std::vector<unsigned long> resize_block(std::vector<unsigned long> input)
	{
		std::vector<unsigned long> output(16);

		// Loop through the 64 sections by 4 steps and merge those 4 sections.
		for (int i = 0; i < 64; i = i + 4)
		{
			std::bitset<32> temp(0);

			// Shift the blocks to their assigned spots and OR them with the original to combine them.
			temp = (unsigned long)input[i] << 24;
			temp |= (unsigned long)input[i + 1] << 16;
			temp |= (unsigned long)input[i + 2] << 8;
			temp |= (unsigned long)input[i + 3];

			// Puts the new 32 bit word into the correct output array location.
			output[i / 4] = temp.to_ulong();
		}

		return output;
	}

	/*
		Shows the current contents of the block in hex.
		This is mainly to avoid having to change the stream from hex to dec and back again in the cout statement which would have been error
		prone.
		Params:
		unsigned long : 32 or less bit block
		Returns:
		std::string : content of the block
	*/
	std::string show_as_hex(unsigned long input)
	{
		std::bitset<32> bs(input);
		unsigned n = bs.to_ulong();

		std::stringstream sstream;
		sstream << std::hex << std::setw(8) << std::setfill('0') << n;
		std::string temp;
		sstream >> temp;

		return temp;
	}

	/*
		Takes the string and convers all characters to their ASCII Binary equivilents.
		Params:
		std::string : any string
		Returns:
		std::vector<unsigned long> : A vector consisting of one 8 bit value per ASCII character.
	*/
	std::vector<unsigned long> convert_to_binary(const std::string input)
	{
		// Vector to hold all of the ASCII character values.
		std::vector<unsigned long> block;

		// For each character, convert the ASCII chararcter to its binary representation.
		for (int i = 0; i < input.size(); ++i)
		{
			// Temporary variable called B to store the 8 bit pattern for the ASCII value.
			std::bitset<8> b(input.c_str()[i]);

			block.push_back(b.to_ulong());
		}

		return block;
	}

	/*
		Takes the vector of ASCII values in binary and pad it so that there will be a
		total of 512 bits.
		Padding specifically consist of having your message, adding 1, and adding 0's to
		it so you will have the current length be 448 bits long, and then adding 64 bits
		which say how long the original message was, giving you a total of 512 bits.
		Input : The message in the form of a vector containing 8 bit binary ASCII values.
		Output : A padded vector consisting of one 8 bit value per ASCII character.
	*/
	std::vector<unsigned long> pad_to_512bits(std::vector<unsigned long> block)
	{
		// l will represent the length of the message in terms of bits, with each
		// block representing one 8 bit ascii character.
		int l = block.size() * 8;

		// Equation for padding is l + 1 + k = 448 mod 512
		// Simplified to: l + 1 + k = 448
		//		  448 - 1 - l = k
		//		  447 - l = k
		// l = length of message in bits
		// k = how much zero's to add so new message will be a multiple of 512.
		int k = 447 - l;

		// Add in another 8 bit block with the first bit set to 1

		unsigned long t1 = 0x80;
		block.push_back(t1);

		// Added in 7 zero's previously, so subtract 7 from k.
		k = k - 7;

		// Add 8 bit blocks containing zero's
		for (int i = 0; i < k / 8; i++)
			block.push_back(0x00000000);

		// 488 bits out of 512. Add l in the binary form of eight 8 bit blocks.
		std::bitset<64> big_64bit_blob(l);

		// Split up that 64 bit blob into 8 bit sections.
		std::string big_64bit_string = big_64bit_blob.to_string();

		// Push the first block into the 56th position.
		std::bitset<8> temp_string_holder1(big_64bit_string.substr(0, 8));
		block.push_back(temp_string_holder1.to_ulong());

		// Push all the rest of the 8 bit blocks in.
		for (int i = 8; i < 63; i = i + 8)
		{
			std::bitset<8> temp_string_holder2(big_64bit_string.substr(i, 8));
			block.push_back(temp_string_holder2.to_ulong());
		}

		return block;
	}

	/*
		Input : The 512 bit padded message as a vector containing 8 bit binary ASCII values.
		Output : A string representing the hash.
	*/
	std::string compute_hash(const std::vector<unsigned long> block)
	{
		// Taken from the spec
		//   SHA-224 and SHA-256 use the same sequence of sixty-four constant
		//   32-bit words, K0, K1, ..., K63.  These words represent the first 32
		//   bits of the fractional parts of the cube roots of the first sixty-
		//   four prime numbers.
		unsigned long k[64] = {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
							   0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
							   0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
							   0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
							   0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
							   0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
							   0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
							   0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

		// Initial Hash Values, first thirty-two bits of the fractional parts of the square roots of the first eight prime numbers.
		unsigned long Hl[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

		unsigned long W[64];

		for (int t = 0; t <= 15; t++)
		{
			W[t] = block[t] & 0xFFFFFFFF;
		}

		for (int t = 16; t <= 63; t++)
		{
			W[t] = SSIG1(W[t - 2]) + W[t - 7] + SSIG0(W[t - 15]) + W[t - 16];

			// Have to make sure we are still dealing with 32 bit numbers.
			W[t] = W[t] & 0xFFFFFFFF;
		}

		unsigned long temp1;
		unsigned long temp2;
		unsigned long a = Hl[0];
		unsigned long b = Hl[1];
		unsigned long c = Hl[2];
		unsigned long d = Hl[3];
		unsigned long e = Hl[4];
		unsigned long f = Hl[5];
		unsigned long g = Hl[6];
		unsigned long h = Hl[7];

		for (int t = 0; t < 64; t++)
		{
			temp1 = h + EP1(e) + CH(e, f, g) + k[t] + W[t];
			temp2 = EP0(a) + MAJ(a, b, c);

			h = g;
			g = f;
			f = e;
			e = (d + temp1) & 0xFFFFFFFF; // Makes sure that we are still using 32 bits.
			d = c;
			c = b;
			b = a;
			a = (temp1 + temp2) & 0xFFFFFFFF; // Makes sure that we are still using 32 bits.
		}

		// Add up all the working variables to each hash and make sure we are still
		// working with solely 32 bit variables.
		unsigned long mashedHl[8] = {
			(Hl[0] + a) & 0xFFFFFFFF,
			(Hl[1] + b) & 0xFFFFFFFF,
			(Hl[2] + c) & 0xFFFFFFFF,
			(Hl[3] + d) & 0xFFFFFFFF,
			(Hl[4] + e) & 0xFFFFFFFF,
			(Hl[5] + f) & 0xFFFFFFFF,
			(Hl[6] + g) & 0xFFFFFFFF,
			(Hl[7] + h) & 0xFFFFFFFF};

		// Append the hash segments together one after the other to get the full
		// 256 bit hash.
		return show_as_hex(mashedHl[0]) + show_as_hex(mashedHl[1]) + show_as_hex(mashedHl[2]) + show_as_hex(mashedHl[3]) +
			   show_as_hex(mashedHl[4]) + show_as_hex(mashedHl[5]) + show_as_hex(mashedHl[6]) + show_as_hex(mashedHl[7]);
	}

  public:
	std::string hash;
	SHA256(std::string s)
	{
		std::vector<unsigned long> block;
		block = convert_to_binary(s);
		block = pad_to_512bits(block);
		block = resize_block(block);
		hash = compute_hash(block);
	}
};

// string function NSBasicHash( string s )
SQRESULT SQ_BasicHash(void* sqvm)
{
	const char* s = ClientSq_getstring(sqvm, 1);
	std::size_t h = std::hash<std::string> {}(s);
	ClientSq_pushstring(sqvm, std::to_string(h).c_str(), -1);

	return SQRESULT_NOTNULL;
}

// string function NSSHA256( string s)
SQRESULT SQ_SHA256(void* sqvm)
{
	const char* s = ClientSq_getstring(sqvm, 1); // PARAM CANNOT BE LONGER THAN 55 CHARS
	if (strlen(s) > 55)
		ClientSq_pusherror(sqvm, "String to be hashed can't be longer than 55 characters");
	else
		ClientSq_pushstring(sqvm, SHA256(s).hash.c_str(), -1);

	return SQRESULT_NOTNULL;
}

void InitialiseScriptHashingClientAndUI(HMODULE baseAddress)
{
	g_ClientSquirrelManager->AddFuncRegistration("string", "NSBasicHash", "string s", "", SQ_BasicHash);
	g_ClientSquirrelManager->AddFuncRegistration("string", "NSSHA256", "string s", "", SQ_SHA256);
	g_UISquirrelManager->AddFuncRegistration("string", "NSBasicHash", "string s", "", SQ_BasicHash);
	g_UISquirrelManager->AddFuncRegistration("string", "NSSHA256", "string s", "", SQ_SHA256);
}

void InitialiseScriptHashingServer(HMODULE baseAddress)
{
	g_ServerSquirrelManager->AddFuncRegistration("string", "NSBasicHash", "string s", "", SQ_BasicHash);
	g_ServerSquirrelManager->AddFuncRegistration("string", "NSSHA256", "string s", "", SQ_SHA256);
}
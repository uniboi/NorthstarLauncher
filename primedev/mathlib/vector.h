#pragma once

#include <cmath>

union Vector3
{
	struct
	{
		float x;
		float y;
		float z;
	};

	float raw[3];

	void MakeValid()
	{
		for (auto& fl : raw)
			if (std::isnan(fl))
				fl = 0;
	}

	// todo: more operators maybe
	bool operator==(const Vector3& other)
	{
		return x == other.x && y == other.y && z == other.z;
	}
};

inline Vector3 StringToVector(char* pString)
{
	Vector3 vRet;

	int length = 0;
	while (pString[length])
	{
		if ((pString[length] == '<') || (pString[length] == '>'))
			pString[length] = '\0';
		length++;
	}

	int startOfFloat = 1;
	int currentIndex = 1;

	while (pString[currentIndex] && (pString[currentIndex] != ','))
		currentIndex++;
	pString[currentIndex] = '\0';
	vRet.x = std::stof(&pString[startOfFloat]);
	startOfFloat = ++currentIndex;

	while (pString[currentIndex] && (pString[currentIndex] != ','))
		currentIndex++;
	pString[currentIndex] = '\0';
	vRet.y = std::stof(&pString[startOfFloat]);
	startOfFloat = ++currentIndex;

	while (pString[currentIndex] && (pString[currentIndex] != ','))
		currentIndex++;
	pString[currentIndex] = '\0';
	vRet.z = std::stof(&pString[startOfFloat]);
	startOfFloat = ++currentIndex;

	return vRet;
}

union QAngle
{
	struct
	{
		float x;
		float y;
		float z;
		float w;
	};

	float raw[4];

	// todo: more operators maybe
	bool operator==(const QAngle& other)
	{
		return x == other.x && y == other.y && z == other.z && w == other.w;
	}
};

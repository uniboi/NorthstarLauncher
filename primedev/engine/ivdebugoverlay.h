#pragma once

#include "mathlib/vector.h"
#include "mathlib/matrix.h"

class IVDebugOverlay
{
  public:
	virtual void AddEntityTextOverlay(int iEntIndex, int iLineOffset, float fDuration, int r, int g, int b, int a, const char* fmt, ...) = 0;

	virtual void AddBoxOverlay(const Vector3& origin, const Vector3& mins, const Vector3& max, QAngle const& orientation, int r, int g, int b, int a, bool doDepthTest, float duration) = 0;
	virtual void AddSphereOverlay(const Vector3& vOrigin, float flRadius, int nTheta, int nPhi, int r, int g, int b, int a, float flDuration) = 0;
	virtual void AddTriangleOverlay(const Vector3& p1, const Vector3& p2, const Vector3& p3, int r, int g, int b, int a, bool doDepthTest, float duration) = 0;
	virtual void AddLineOverlay(const Vector3& origin, const Vector3& dest, int r, int g, int b, bool doDepthTest, float duration) = 0;
	virtual void sub_1800AA120(int a2, int a3, int a4, int a5, int a6, char a7, int a8) = 0;

	virtual void AddTextOverlay(__int64 a2, __int64 a3, __int64 a4, const char* a5, ...) = 0;
	virtual void AddTextOverlay(__int64 a2, __int64 a3, const char* a4, ...) = 0;

	virtual void sub_1800AA1B0() = 0;
	virtual void sub_1800AA210() = 0;

	virtual void AddSweptBoxOverlay(const Vector3& start, const Vector3& end, const Vector3& mins, const Vector3& max, const QAngle& angles, int r, int g, int b, int a, float flDuration) = 0;
	virtual void AddGridOverlay(const Vector3& vPos) = 0;
	virtual void AddCoordFrameOverlay(const matrix3x4_t& frame, float flScale, int vColorTable[3][3] = NULL) = 0; // Untested

	virtual void sub_1800AC1B0() = 0;
	virtual void sub_1800AC280() = 0;
	virtual void sub_1800ADE20() = 0;
	virtual void sub_1800AAB90() = 0;
	virtual void sub_1800AAA60() = 0;
	virtual void sub_1800AB670() = 0;
	virtual void sub_1800ADEC0() = 0;
	virtual void sub_1800ABDD0() = 0;
	virtual void sub_1800ADE80() = 0;

	virtual void AddTextOverlayRGB(const Vector3& origin, int line_offset, float duration, float r, float g, float b, float alpha, const char* format, ...) = 0;
	virtual void AddTextOverlayRGB(const Vector3& origin, int line_offset, float duration, int r, int g, int b, int a, const char* format, ...) = 0;

	virtual void sub_1800A9F00(void* a2, void* a3, int a4, int a5, int a6, int a7, char a8) = 0;
	virtual void sub_1800A9870(void* a2, void* a3, void* a4, void* a5, void* a6, void* a7) = 0;

	virtual void sub_1800AD520() = 0;
	virtual void sub_1800AC180() = 0;
	virtual void sub_1800ADF70() = 0;
	virtual void sub_1800AC260() = 0;
	virtual void sub_1800ACC00() = 0;
};

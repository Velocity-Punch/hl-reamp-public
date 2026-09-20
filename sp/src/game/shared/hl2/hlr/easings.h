#ifndef EASINGS
#define EASINGS

#ifdef _WIN32
#pragma once
#endif


#include <mathlib/mathlib.h>

inline float EaseOutLight(float frac)
{
	return sinf((frac * M_PI_F) * 0.5f);
}
inline float EaseInLight(float frac)
{
	return 1.f - cosf((frac * M_PI_F) / 2.f);
}
inline float EaseInOutLight(float frac)
{
	float rtrn;
	if (frac < 0.5f)
		rtrn = EaseInLight(frac);
	else
		rtrn = EaseOutLight(frac);

	return rtrn;
}

inline float EaseInMedium(float frac)
{
	return frac * frac;
}

inline float EaseOutMedium(float frac)
{
	return 1.f - (1.f - frac) * (1.f - frac);
}

inline float EaseInOutMedium(float frac)
{
	float rtrn;
	if (frac < 0.5f)
		rtrn = 2.f * EaseInMedium(frac);
	else
	{
		float div = (-2.f * frac + 2.f);
		rtrn = 1.f - (div * div * 0.5f);
	}
	return rtrn;
}

inline float EaseInHeavy(float frac)
{
	return frac * frac * frac;
}
inline float EaseOutHeavy(float frac)
{
	float inv = 1.f - frac;
	return 1.f - inv * inv * inv;
}
inline float EaseInOutHeavy(float frac)
{
	float rtrn;
	if (frac < 0.5f)
		rtrn = EaseInHeavy(frac) * 4.f;
	else
	{
		float div = (-2.f * frac + 2.f);
		rtrn = 1.f - (div * div * div) * 0.5f;
	}

	return rtrn;
}

inline float EaseInExtreme(float frac)
{
	return frac * frac * frac * frac;
}
inline float EaseOutExtreme(float frac)
{
	float inv = 1.f - frac;
	return 1.f - inv * inv * inv * inv;
}
inline float EaseInOutExtreme(float frac)
{
	float rtrn;
	if (frac < 0.5f)
		rtrn = EaseInExtreme(frac) * 8.f;
	else
	{
		float div = (-2.f * frac + 2.f);
		rtrn = 1.f - div * div * div * div * 0.5f;
	}
	return rtrn;
}
inline float EaseInCustom(float frac, int strength)
{
	int str = Clamp(strength, 1, 4);

	switch (str)
	{
	case 1:
		return EaseInLight(frac);
		break;
	case 2:
		return EaseInMedium(frac);
		break;
	case 3:
		return EaseInHeavy(frac);
		break;
	case 4:
		return EaseInExtreme(frac);
		break;
	default:
		return frac;
		break;
	}
}

inline float EaseOutCustom(float frac, int strength)
{
	int str = Clamp(strength, 1, 4);

	switch (str)
	{
	case 1:
		return EaseOutLight(frac);
		break;
	case 2:
		return EaseOutMedium(frac);
		break;
	case 3:
		return EaseOutHeavy(frac);
		break;
	case 4:
		return EaseOutExtreme(frac);
		break;
	default:
		return frac;
		break;
	}
}
inline float EaseInOutCustom(float frac, int strength)
{
	int str = Clamp(strength, 1, 4);

	switch (str)
	{
	case 1:
		return EaseInOutLight(frac);
		break;
	case 2:
		return EaseInOutMedium(frac);
		break;
	case 3:
		return EaseInOutHeavy(frac);
		break;
	case 4:
		return EaseInOutExtreme(frac);
		break;
	default:
		return frac;
		break;
	}
}
#endif //EASINGS



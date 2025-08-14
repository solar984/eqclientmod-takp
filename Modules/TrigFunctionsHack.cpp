#include "eqclientmod.h"
// this hack substitutes more precise versions of trigonometry functions in place of some 'fast' ones

#ifdef TRIGFUNCTIONS_HACK
#include "common.h"
#include "util.h"
#include "settings.h"

typedef float(__cdecl *_FastMathFunction)(float);
_FastMathFunction GetFastCosine_Trampoline;
_FastMathFunction GetFastSine_Trampoline;
_FastMathFunction GetFastCotangent_Trampoline;

typedef float(__cdecl *_CalculateCoefficientsFromHeadingPitchRoll)(float, float, float, float);
_CalculateCoefficientsFromHeadingPitchRoll CalculateCoefficientsFromHeadingPitchRoll_Trampoline;
typedef float(__cdecl *_CalculateHeadingPitchRollFromCoefficients)(float, float, float, float);
_CalculateHeadingPitchRollFromCoefficients CalculateHeadingPitchRollFromCoefficients_Trampoline;

DWORD org_nonFastCos = 0;
DWORD org_nonFastSin = 0;
DWORD org_nonFastCotangent = 0;
DWORD org_fix16Tangent = 0;
DWORD org_calculateAccurateCoefficientsFromHeadingPitchRoll = 0;
DWORD org_calculateAccurateHeadingPitchRollFromCoefficients = 0;

float __cdecl t3dFastCosine_Detour(float a1)
{
	if (org_nonFastCos)
	{
		return ((float(__cdecl *) (float)) org_nonFastCos)(a1);
	}

	return GetFastCosine_Trampoline(a1);
}

float __cdecl t3dFastSine_Detour(float a1)
{
	if (org_nonFastSin)
	{
		return ((float(__cdecl *) (float)) org_nonFastSin)(a1);
	}

	return GetFastSine_Trampoline(a1);
}

float __cdecl t3dFastCotangent_Detour(float a1)
{
	if (org_nonFastCotangent)
	{
		return ((float(__cdecl *) (float)) org_nonFastCotangent)(a1);
	}

	return GetFastCotangent_Trampoline(a1);
}

float CalculateCoefficientsFromHeadingPitchRoll_Detour(float a1, float a2, float a3, float a4)
{
	if (org_calculateAccurateCoefficientsFromHeadingPitchRoll)
	{
		return ((float(__cdecl *) (float, float, float, float))org_calculateAccurateCoefficientsFromHeadingPitchRoll)(a1, a2, a3, a4);
	}

	return CalculateCoefficientsFromHeadingPitchRoll_Trampoline(a1, a2, a3, a4);
}

float CalculateHeadingPitchRollFromCoefficients_Detour(float a1, float a2, float a3, float a4)
{
	if (org_calculateAccurateHeadingPitchRollFromCoefficients)
	{
		return ((float(__cdecl *) (float, float, float, float))org_calculateAccurateHeadingPitchRollFromCoefficients)(a1, a2, a3, a4);
	}

	return CalculateHeadingPitchRollFromCoefficients_Trampoline(a1, a2, a3, a4);
}

void LoadTrigFunctionsHack()
{
	bool enable = true;

#ifdef INI_FILE
	char buf[2048];
	const char *desc = "This hack improves the quality of the trigonometry functions used in some places, smoothing out mouse control.";
	WritePrivateProfileStringA("TrigFunctions", "Description", desc, INI_FILE);
	GetINIString("TrigFunctions", "Enabled", "TRUE", buf, sizeof(buf), true);
	enable = ParseINIBool(buf);
#endif

	Log("LoadTrigFunctionsHack(): hack is %s", enable ? "ENABLED" : "DISABLED");

	if (enable)
	{
		_FastMathFunction ot3dFastCosine = (_FastMathFunction)GetProcAddress(hEQGfxDll, "t3dFloatFastCosine");
		if (ot3dFastCosine)
		{
			(_FastMathFunction)GetFastCosine_Trampoline = (_FastMathFunction)Detour((PBYTE)ot3dFastCosine, (PBYTE)t3dFastCosine_Detour);
		}

		_FastMathFunction ot3dFastSine = (_FastMathFunction)GetProcAddress(hEQGfxDll, "t3dFloatFastSine");
		if (ot3dFastSine)
		{
			(_FastMathFunction)GetFastSine_Trampoline = (_FastMathFunction)Detour((PBYTE)ot3dFastSine, (PBYTE)t3dFastSine_Detour);
		}

		_FastMathFunction ot3dFastCotangent = (_FastMathFunction)GetProcAddress(hEQGfxDll, "t3dFloatFastCotangent");
		if (ot3dFastCotangent)
		{
			(_FastMathFunction)GetFastCotangent_Trampoline = (_FastMathFunction)Detour((PBYTE)ot3dFastCotangent, (PBYTE)t3dFastCotangent_Detour);
		}

		_CalculateCoefficientsFromHeadingPitchRoll ot3dCalculateCoefficientsFromHeadingPitchRoll = (_CalculateCoefficientsFromHeadingPitchRoll)GetProcAddress(hEQGfxDll, "t3dCalculateCoefficientsFromHeadingPitchRoll");
		if (ot3dCalculateCoefficientsFromHeadingPitchRoll)
		{
			(_CalculateCoefficientsFromHeadingPitchRoll)CalculateCoefficientsFromHeadingPitchRoll_Trampoline = (_CalculateCoefficientsFromHeadingPitchRoll)Detour((PBYTE)ot3dCalculateCoefficientsFromHeadingPitchRoll, (PBYTE)CalculateCoefficientsFromHeadingPitchRoll_Detour);
		}

		_CalculateHeadingPitchRollFromCoefficients ot3dCCalculateHeadingPitchRollFromCoefficients = (_CalculateHeadingPitchRollFromCoefficients)GetProcAddress(hEQGfxDll, "t3dCalculateHeadingPitchRollFromCoefficients");
		if (ot3dCCalculateHeadingPitchRollFromCoefficients)
		{
			(_CalculateHeadingPitchRollFromCoefficients)CalculateHeadingPitchRollFromCoefficients_Trampoline = (_CalculateHeadingPitchRollFromCoefficients)Detour((PBYTE)ot3dCCalculateHeadingPitchRollFromCoefficients, (PBYTE)CalculateHeadingPitchRollFromCoefficients_Detour);
		}

		org_nonFastCos = (DWORD)GetProcAddress(hEQGfxDll, "t3dFloatCosine");
		org_nonFastSin = (DWORD)GetProcAddress(hEQGfxDll, "t3dFloatSine");
		org_nonFastCotangent = (DWORD)GetProcAddress(hEQGfxDll, "t3dFloatCotangent");
		org_calculateAccurateCoefficientsFromHeadingPitchRoll = (DWORD)GetProcAddress(hEQGfxDll, "t3dCalculateAccurateCoefficientsFromHeadingPitchRoll");
		org_calculateAccurateHeadingPitchRollFromCoefficients = (DWORD)GetProcAddress(hEQGfxDll, "t3dCalculateAccurateHeadingPitchRollFromCoefficients");
	}
}

#endif

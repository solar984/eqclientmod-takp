#include "eqclientmod.h"

#ifdef NAMEPLATE_HACK
#include "util.h"
#include "common.h"
#include "settings.h"
#include "eq_functions.h"

int NP_MaxScaleFactor_Enable = 1;
//float NP_MaxScaleFactor_Height = 0.012f; // default 0.050000001f
//float NP_SizeMulti = 1.4f; // default 1.0f
float NP_MaxScaleFactor_Height = 0.019f; // default 0.050000001f
float NP_SizeMulti = 1.0f; // default 1.0f
int NP_OfsY = 0;
//int NP_16 = 16; // 0 = left align, 8 = right align, 16 = center

// s3dSetStringSpriteYonClip
typedef int(__cdecl *_s3dSetStringSpriteYonClip)(intptr_t, int, float);
_s3dSetStringSpriteYonClip s3dSetStringSpriteYonClip_Trampoline;
int __cdecl s3dSetStringSpriteYonClip_Detour(intptr_t sprite, int a2, float distance)
{
	//Log("s3dSetStringSpriteYonClip_Detour 0x%lx %d %f", sprite, a2, distance);

	if ((*(unsigned int *)&distance) == 0x428c0000) // 70.0f
	{
		a2 = 0;
		//distance = 1000.0f;
	}

	return s3dSetStringSpriteYonClip_Trampoline(sprite, a2, distance);
}

//s3dSetStringSpriteMaxScaleFactor
typedef int(__cdecl *_s3dSetStringSpriteMaxScaleFactor)(intptr_t, int, float);
_s3dSetStringSpriteMaxScaleFactor s3dSetStringSpriteMaxScaleFactor_Trampoline;
int __cdecl s3dSetStringSpriteMaxScaleFactor_Detour(intptr_t sprite, int a2, float scale)
{
	Log("s3dSetStringSpriteMaxScaleFactor_Detour 0x%lx %d %f", sprite, a2, scale);

	if ((*(unsigned int *)&scale) == 0x3D4CCCCD) // 0.050000001f
	{
		Log("s3dSetStringSpriteMaxScaleFactor_Detour overriding scale %f", NP_MaxScaleFactor_Height);
		a2 = NP_MaxScaleFactor_Enable;
		scale = NP_MaxScaleFactor_Height;
	}

	return s3dSetStringSpriteMaxScaleFactor_Trampoline(sprite, a2, scale);
}

//s3dCalcStringSpriteDimensions
typedef int(__cdecl *_s3dCalcStringSpriteDimensions)(intptr_t sprite);
_s3dCalcStringSpriteDimensions s3dCalcStringSpriteDimensions_Trampoline;
int __cdecl s3dCalcStringSpriteDimensions_Detour(intptr_t sprite)
{
	int *width = (int *)((char *)sprite + 60);
	int *height = (int *)((char *)sprite + 64);

	int ret = s3dCalcStringSpriteDimensions_Trampoline(sprite);
	*width = (int)(NP_SizeMulti * *width);
	*height = (int)(NP_SizeMulti * *height);
	Log("s3dCalcStringSpriteDimensions_Detour3 : %d %d", *width, *height);

	return ret;
}

#if false
static uint32_t rgb(double ratio)
{
	//we want to normalize ratio so that it fits in to 6 regions
	//where each region is 256 units long
	int normalized = (int)(ratio * 256 * 6);

	//find the region for this position
	int region = normalized / 256;

	//find the distance to the start of the closest region
	int ofs = normalized % 256;

	uint8_t r = 0, g = 0, b = 0;
	switch (region)
	{
	case 0: r = 255; g = 0;   b = 0;   g += ofs; break;
	case 1: r = 255; g = 255; b = 0;   r -= ofs; break;
	case 2: r = 0;   g = 255; b = 0;   b += ofs; break;
	case 3: r = 0;   g = 255; b = 255; g -= ofs; break;
	case 4: r = 0;   g = 0;   b = 255; r += ofs; break;
	case 5: r = 255; g = 0;   b = 255; b -= ofs; break;
	}
	return r + (g << 8) + (b << 16) + 0xff000000;
}
#endif

class NP;
typedef int(__thiscall *_CCachedFont__DrawNameTagText_Trampoline)(NP *this_ptr, CXStr *cxstr, int x1, int y1, int x2, int y2, int a7, int a8, float a9);
_CCachedFont__DrawNameTagText_Trampoline CCachedFont__DrawNameTagText_Trampoline;

DWORD DarkenColor(DWORD color)
{
	// ABGR
	DWORD out_color = color & 0xFF000000;

	for (int i = 0; i < 3; i++)
	{
		uint8_t c = (color >> (8 * i)) & 0xFF;
		c /= 3;
		out_color |= (c << (8 * i));
	}

	return out_color;
}

class NP
{
public:
	int CCachedFont__DrawNameTagText_Detour(CXStr *cxstr, int x1, int y1, int x2, int y2, DWORD tint, int align, float occ_depth)
	{
		char cxstrbuf[24] = { 0 };
		CXStr *cxstrcpy = (CXStr *)&cxstrbuf;

		// it's a UTF-16 string, assume it's ASCII compatible and skip the zeroes
		/*
		char dest[200] = { 0 };
		char *s = (char *)cxstr + 20;
		for (int i = 0; i < 199; s++)
		{
			if (*s)
			{
				dest[i++] = *s;
			}
			else if (*(s + 1) == 0)
			{
				dest[i++] = 0;
				break;
			}
		}
		*/

		// RGB effect
#if false
		static int rainbow = 0;
		uint32 *cur_gametime = (uint32 *)(*(intptr_t *)0x007F9510 + 200);
		static uint32 last_gametime = *cur_gametime;
		float add = (*cur_gametime - last_gametime) * 1.0f;
		last_gametime = *cur_gametime;
		int cycle_scale = 10000;
		tint = rgb(rainbow / (float)cycle_scale);
		rainbow = (int)(rainbow + add);
		if (rainbow >= cycle_scale)
			rainbow = 0;
#endif

		//Log("CCachedFont__DrawNameTagText_Detour \"%s\" %d %d %d %d (%d x %d) - 0x%lx %d %f", dest, x1, y1, x2, y2, x2 - x1, y2 - y1, a7, a8, a9);

#if true
		// drop shadow
		// make a copy of the string for the drop shadow call, the function destroys the CXStr so can't just use the same one twice
		* (CXStr *)&cxstrcpy = *(CXStr *)&cxstr;
		CCachedFont__DrawNameTagText_Trampoline(this, cxstrcpy, x1 + 1, y1 + 1 + NP_OfsY, x2 + 1, y2 + 1 + NP_OfsY, DarkenColor(tint), align, occ_depth);

		return CCachedFont__DrawNameTagText_Trampoline(this, cxstr, x1, y1 + NP_OfsY, x2, y2 + NP_OfsY, tint, align, occ_depth);
#else
		char str[100];
		snprintf(str, 100, "occ_depth: %f", occ_depth);
		*(CXStr *)&cxstrcpy = str;
		return CCachedFont__DrawNameTagText_Trampoline(this, cxstrcpy, x1, y1, x2, y2, tint, align, 0);
#endif
	}
};


/*
#define EQ_FUNCTION_AT_VARIABLE_ADDRESS(function,variable) __declspec(naked) function\
{\
	__asm{mov eax, [variable]};\
	__asm{jmp eax};\
}
*/

//intptr_t CCachedFont__DrawNameTagText_Offset = 0x77D10;
//EQ_FUNCTION_AT_VARIABLE_ADDRESS(int NP::CCachedFont__DrawNameTagText(int cxstr, int x1, int y1, int x2, int y2, int a7, int a8, float a9), CCachedFont__DrawNameTagText_Offset);

#ifdef COMMAND_HANDLER
void command_np(void *LocalPlayer, char *text, char *cmd, char *&sep)
{
	char buf[150];
	int DisplayObject = *(int *)0x007F9510;
	char *a1 = strtok_s(sep, " ", &sep);
	char *a2 = strtok_s(sep, " ", &sep);
	char *a3 = strtok_s(sep, " ", &sep);

	if (a1 && *a1)
	{
		float val = (float)atof(a1);
		snprintf(buf, 150, "setting NP_SizeMulti to %0.2f", val);
		EverQuestObject->dsp_chat(buf, 281, 0);
		NP_SizeMulti = val;
		if (a2 && *a2)
		{
			float val = (float)atof(a2);
			snprintf(buf, 150, "setting NP_MaxScaleFactor_Height to %0.2f", val);
			EverQuestObject->dsp_chat(buf, 281, 0);
			NP_MaxScaleFactor_Height = val;
			if (a3 && *a3)
			{
				int val = atoi(a3);
				snprintf(buf, 150, "setting NP_OfsY to %d", val);
				EverQuestObject->dsp_chat(buf, 281, 0);
				NP_OfsY = val;
			}
		}
		/*
		float xmult = atof(a1);
		snprintf(buf, 150, "setting NP_xmult to %0.2f", xmult);
		EverQuestObject->dsp_chat(buf, 281, 0);
		NP_xmult = xmult;

		if (a2 && *a2)
		{
			float ymult = atof(a2);
			snprintf(buf, 150, "setting NP_ymult to %0.2f", xmult);
			EverQuestObject->dsp_chat(buf, 281, 0);
			NP_ymult = ymult;
		}
		if (a3 && *a3)
		{
			float a9 = atof(a3);
			snprintf(buf, 150, "setting NP_a9 to %0.2f", a9);
			EverQuestObject->dsp_chat(buf, 281, 0);
			NP_a9 = a9;
		}
		*/
	}
	else
	{
		//snprintf(buf, 150, "NP_xmult: %0.2f NP_ymult: %0.2f NP_a9: %0.2f", NP_xmult, NP_ymult, NP_a9);
		snprintf(buf, 150, "NP_SizeMulti: %f NP_MaxScaleFactor_Height: %f NP_OfsY: %d", NP_SizeMulti, NP_MaxScaleFactor_Height, NP_OfsY);
		EverQuestObject->dsp_chat(buf, 281, 0);
	}
}
#endif


void LoadNameplateHack()
{
	bool enable = false;
	bool scalingEnabled = false;

#ifdef INI_FILE
	char buf[2048];
	const char *desc = "Changes nameplate behavior to make them always visible, not hidden with distance.  This is helpful together with the farclip hack.  There is also a scaling option which is useful with the defaultx2 4K UI.";
	WritePrivateProfileStringA("Nameplate", "Description", desc, INI_FILE);
	GetINIString("Nameplate", "Enabled", "FALSE", buf, sizeof(buf), true);
	enable = ParseINIBool(buf);
	GetINIString("Nameplate", "Scaling", "FALSE", buf, sizeof(buf), true);
	scalingEnabled = ParseINIBool(buf);
#endif

	Log("LoadNameplateHack(): hack is %s", enable ? "ENABLED" : "DISABLED");

	if (enable)
	{
		Log("LoadNameplateHack()");

		// this one disables hiding the name tags due to distance
		_s3dSetStringSpriteYonClip s3dSetStringSpriteYonClip = (_s3dSetStringSpriteYonClip)GetProcAddress(hEQGfxDll, "s3dSetStringSpriteYonClip");
		s3dSetStringSpriteYonClip_Trampoline = (_s3dSetStringSpriteYonClip)DetourWithTrampoline(s3dSetStringSpriteYonClip, s3dSetStringSpriteYonClip_Detour, 8);

		if (scalingEnabled)
		{
			//_s3dSetStringSpriteMaxScaleFactor s3dSetStringSpriteMaxScaleFactor = (_s3dSetStringSpriteMaxScaleFactor)GetProcAddress(hEQGfxDll, "s3dSetStringSpriteMaxScaleFactor");
			_s3dSetStringSpriteMaxScaleFactor s3dSetStringSpriteMaxScaleFactor = (_s3dSetStringSpriteMaxScaleFactor)((intptr_t)hEQGfxDll + (0x000216A0));
			s3dSetStringSpriteMaxScaleFactor_Trampoline = (_s3dSetStringSpriteMaxScaleFactor)DetourWithTrampoline(s3dSetStringSpriteMaxScaleFactor, s3dSetStringSpriteMaxScaleFactor_Detour, 10);

			// this multiplier hack makes the names bigger at a distance but it messes up the position when close
			//_s3dCalcStringSpriteDimensions s3dCalcStringSpriteDimensions = (_s3dCalcStringSpriteDimensions)GetProcAddress(hEQGfxDll, "s3dCalcStringSpriteDimensions");
			_s3dCalcStringSpriteDimensions s3dCalcStringSpriteDimensions = (_s3dCalcStringSpriteDimensions)((intptr_t)hEQGfxDll + (0x000212C0));
			//s3dCalcStringSpriteDimensions_Trampoline = (_s3dCalcStringSpriteDimensions)DetourWithTrampoline(s3dCalcStringSpriteDimensions, s3dCalcStringSpriteDimensions_Detour, 5);


			// this one is the actual drawing of the nametag after it's deferred, can alter the final size and content here
			// .text:10070E0D E8 FE 6E 00 00                                call    CCachedFont__DrawNameTagText
			MethodAddressToVariable(NP_CCachedFont__DrawNameTagText_Detour, NP::CCachedFont__DrawNameTagText_Detour);
			CCachedFont__DrawNameTagText_Trampoline = (_CCachedFont__DrawNameTagText_Trampoline)DetourWithTrampoline((void *)(intptr_t)(0x77D10 + (intptr_t)hEQGfxDll), (void *)NP_CCachedFont__DrawNameTagText_Detour, 7);

			//float *s3d_gfFontToWorldSpaceScale = (float *)(0xDB1A8 + hEQGfxDll); // 0.0500000007451
			//float val = NP_MaxScaleFactor_Height;
			//Patch(s3d_gfFontToWorldSpaceScale, &val, 4);
		}

#ifdef FONT_HACK
		// override font for name sprites only by sending a magic value that's recognized in the other hack and adjusted there
		// .text:004A8FD4 8A8 6A 14                                         push    20              ; _DWORD
		{
			char val = 69;
			Patch((void *)(0x004A8FD4 + 1), &val, 1);
		}
#endif

#ifdef COMMAND_HANDLER
		ChatCommandMap["/np"] = command_np;
#endif
	}
}
#endif

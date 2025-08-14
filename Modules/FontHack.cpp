#include "eqclientmod.h"

#ifdef FONT_HACK
#include "common.h"
#include "util.h"
#include "settings.h"
#include "eq_functions.h"
#include <string>

//#define EQ_POINTER_CEverQuest 0x00809478

#if false
// this only gets called in the stone UI
HFONT __cdecl MakeFont_Detour(int lfHeight, int lfWeight, LPCSTR lfFaceName)
{
	HFONT hf; // esi
	LOGFONTA lf;

	memset(&lf, 0, sizeof(lf));
	lf.lfHeight = lfHeight;
	lf.lfWeight = lfWeight;
	lf.lfQuality = 3;
	lf.lfCharSet = 1;
	lstrcpyA(lf.lfFaceName, lfFaceName);

	//
	// these work without glitching. width calc problems?
	//
	// Palatino Linotype - best one, works well at 32 height 400/700 weight
	//
	// Courier New - nice, fixed width, ends up a bit wide.  size 30 weight 700
	// Consolas - nicer than Courier New, similar.  size 30, weight 600 is good for chat and inventory windows
	// Georgia - decent, less wide than Consolas but the numbers are offset weird
	// Ink Free - hard to read
	// Lucida Console - kind of wide
	//

	std::string outstring;
	outstring = "MakeFont_Detour  LOGFONTA *lplf: ";
	outstring += " lfHeight = " + std::to_string(lf.lfHeight);
	outstring += " lfWidth = " + std::to_string(lf.lfWidth);
	outstring += " lfEscapement = " + std::to_string(lf.lfEscapement);
	outstring += " lfOrientation = " + std::to_string(lf.lfOrientation);
	outstring += " lfWeight = " + std::to_string(lf.lfWeight);
	outstring += " lfItalic = " + std::to_string(lf.lfItalic);
	outstring += " lfUnderline = " + std::to_string(lf.lfUnderline);
	outstring += " lfStrikeOut = " + std::to_string(lf.lfStrikeOut);
	outstring += " lfCharSet = " + std::to_string(lf.lfCharSet);
	outstring += " lfOutPrecision = " + std::to_string(lf.lfOutPrecision);
	outstring += " lfClipPrecision = " + std::to_string(lf.lfClipPrecision);
	outstring += " lfQuality = " + std::to_string(lf.lfQuality);
	outstring += " lfPitchAndFamily = " + std::to_string(lf.lfPitchAndFamily);
	outstring += " lfFaceName = " + static_cast<std::string>(lf.lfFaceName);
	Log(outstring.c_str());

	if (*(PBYTE **)EQ_POINTER_CEverQuest) // don't do it at login
	{
		if (lf.lfHeight == 10 && lf.lfWeight == 100) // Font 0 - not used in the UI
		{
			lf.lfHeight = 24;
			lf.lfWeight = 400;
			strcpy(lf.lfFaceName, "Palatino Linotype");
		}
		else if (lf.lfHeight == 12 && lf.lfWeight == 100) // Font 1
		{
			lf.lfHeight = 25;
			lf.lfWeight = 400;
			strcpy(lf.lfFaceName, "Palatino Linotype");
		}
		else if (lf.lfHeight == 14 && lf.lfWeight == 100) // Font 2
		{
			lf.lfHeight = 26;
			lf.lfWeight = 400;
			strcpy(lf.lfFaceName, "Palatino Linotype");
		}
		else if (lf.lfHeight == 15 && lf.lfWeight == 100) // Font 3 - used for many UI elements in the default UI, like titles and button labels
		{
			lf.lfHeight = 28;
			lf.lfWeight = 400;
			strcpy(lf.lfFaceName, "Palatino Linotype");
		}
		else if (lf.lfHeight == 16 && lf.lfWeight == 100) // Font 4
		{
			lf.lfHeight = 28;
			lf.lfWeight = 700;
			strcpy(lf.lfFaceName, "Palatino Linotype");
		}
		else if (lf.lfHeight == 20 /*&& lf.lfWeight == 700*/) // Font 5 - also character nametag above head
		{
			lf.lfHeight = 30;
			//lf.lfHeight = 40;
			lf.lfWeight = 700;
			lf.lfQuality = 4;
			strcpy(lf.lfFaceName, "Palatino Linotype");
		}
		else if (lf.lfHeight == 24 && lf.lfWeight == 700) // Font 6 - seems to be used for YOU HAVE BEEN DISCONNECTED too
		{
			lf.lfHeight = 32;
			lf.lfWeight = 700;
			lf.lfQuality = 4;
			strcpy(lf.lfFaceName, "Palatino Linotype");
		}
	}
	//

	hf = CreateFontIndirectA(&lf);
	if (!hf) Log("MakeFont_Detour: * * * * * * * ERROR: UNABLE TO CREATE FONT FOR %s, %ld, %ld!!!\n", lfFaceName, lfHeight, lfWeight);

	return hf;
}
#endif

#if false
// i think these fonts are only used in the stone UI and in the netstat overlay?
HFONT WINAPI CreateFontA_Detour(int cHeight, int cWidth, int cEscapement, int cOrientation, int cWeight, DWORD bItalic, DWORD bUnderline, DWORD bStrikeOut, DWORD iCharSet, DWORD iOutPrecision, DWORD iClipPrecision, DWORD iQuality, DWORD iPitchAndFamily, LPCSTR pszFaceName)
{
	std::string outstring;
	outstring = "CreateFontA_Detour: ";
	outstring += " cHeight = " + std::to_string(cHeight);
	outstring += " cWidth = " + std::to_string(cWidth);
	outstring += " cEscapement = " + std::to_string(cEscapement);
	outstring += " cOrientation = " + std::to_string(cOrientation);
	outstring += " cWeight = " + std::to_string(cWeight);
	outstring += " bItalic = " + std::to_string(bItalic);
	outstring += " bUnderline = " + std::to_string(bUnderline);
	outstring += " bStrikeOut = " + std::to_string(bStrikeOut);
	outstring += " iCharSet = " + std::to_string(iCharSet);
	outstring += " iOutPrecision = " + std::to_string(iOutPrecision);
	outstring += " iClipPrecision = " + std::to_string(iClipPrecision);
	outstring += " iQuality = " + std::to_string(iQuality);
	outstring += " iPitchAndFamily = " + std::to_string(iPitchAndFamily);
	outstring += " pszFaceName = " + static_cast<std::string>(pszFaceName);
	if (iQuality == 4)
	{
		outstring += " called from t3dCreateFont -> CD3DFont::InitDeviceObjects";
	}
	Log(outstring.c_str());

	if (*(PBYTE **)EQ_POINTER_CEverQuest) // don't do it at login
	{
		// all the weights are 400 here - CreateFontA is only called from t3dCreateFont -> CD3DFont::InitDeviceObjects
		// i think these are only used by the stone UI except for the netstat indicator which is used in both
		if (cHeight == 10) // Font 0 - not used in the UI
		{
			cHeight = 24;
		}
		else if (cHeight == 12) // Font 1 - this is used for the netstat indicator in the upper left corner in CreateFontA
		{
			cHeight = 25;
		}
		else if (cHeight == 14) // Font 2
		{
			cHeight = 26;
		}
		else if (cHeight == 15) // Font 3 - used for many UI elements in the default UI, like titles and button labels
		{
			cHeight = 28;
		}
		else if (cHeight == 16) // Font 4
		{
			cHeight = 28;
			//cWeight = 700;
		}
		else if (cHeight == 20) // Font 5 - also character nametag above head
		{
			cHeight = 30;
			//cWeight = 700;
		}
		pszFaceName = "Palatino Linotype";
	}

	return CreateFontA(cHeight, cWidth, cEscapement, cOrientation, cWeight, bItalic, bUnderline, bStrikeOut, iCharSet, iOutPrecision, iClipPrecision, iQuality, iPitchAndFamily, pszFaceName);
}
#endif

HFONT WINAPI CreateFontIndirectA_Detour(LOGFONTA *lplf)
{
	/*
	std::string outstring;
	outstring = "CreateFontIndirectA_Detour  LOGFONTA *lplf: ";
	outstring += " lfHeight = " + std::to_string(lplf->lfHeight);
	outstring += " lfWidth = " + std::to_string(lplf->lfWidth);
	outstring += " lfEscapement = " + std::to_string(lplf->lfEscapement);
	outstring += " lfOrientation = " + std::to_string(lplf->lfOrientation);
	outstring += " lfWeight = " + std::to_string(lplf->lfWeight);
	outstring += " lfItalic = " + std::to_string(lplf->lfItalic);
	outstring += " lfUnderline = " + std::to_string(lplf->lfUnderline);
	outstring += " lfStrikeOut = " + std::to_string(lplf->lfStrikeOut);
	outstring += " lfCharSet = " + std::to_string(lplf->lfCharSet);
	outstring += " lfOutPrecision = " + std::to_string(lplf->lfOutPrecision);
	outstring += " lfClipPrecision = " + std::to_string(lplf->lfClipPrecision);
	outstring += " lfQuality = " + std::to_string(lplf->lfQuality);
	outstring += " lfPitchAndFamily = " + std::to_string(lplf->lfPitchAndFamily);
	outstring += " lfFaceName = " + static_cast<std::string>(lplf->lfFaceName);
	Log(outstring.c_str());
	*/

	// these work without glitching. width calc problems?
	//
	// Palatino Linotype - best one, works well at 32 height 400/700 weight
	//
	// Courier New - nice, fixed width, ends up a bit wide.  size 30 weight 700
	// Consolas - nicer than Courier New, similar.  size 30, weight 600 is good for chat and inventory windows
	// Georgia - decent, less wide than Consolas but the numbers are offset weird
	// Ink Free - hard to read
	// Lucida Console - kind of wide
	//

	if (EverQuestObject) // don't do it at login
	{
		//strcpy(lplf->lfFaceName, "Palatino Linotype");
		//strcpy(lplf->lfFaceName, "Comic Sans MS");
		strcpy(lplf->lfFaceName, "Calibri");
		lplf->lfQuality = 5;
		if (lplf->lfHeight == 10) // Font 0 - not used in the UI
		{
			// weight 100
			lplf->lfHeight = 24;
			lplf->lfWeight = 400;
		}
		else if (lplf->lfHeight == 12) // Font 1
		{
			// weight 100
			lplf->lfHeight = 25;
			lplf->lfWeight = 400;
		}
		else if (lplf->lfHeight == 14) // Font 2
		{
			// weight 100
			lplf->lfHeight = 26;
			lplf->lfWeight = 400;
		}
		else if (lplf->lfHeight == 15) // Font 3 - used for many UI elements in the default UI, like titles and button labels
		{
			// weight 100
			lplf->lfHeight = 28;
			lplf->lfWeight = 400;
		}
		else if (lplf->lfHeight == 16) // Font 4
		{
			// weight 100
			lplf->lfHeight = 28;
			lplf->lfWeight = 700;
		}
		else if (lplf->lfHeight == 20) // Font 5 - also character nametag above head
		{
			// weight 700 and 800 but i think only the 700 weight call's result is used to render
			lplf->lfHeight = 30;
			// if (lplf->lfWeight == 800) // this is the name tag font, created through s3dCreateFontTexture, but doesn't seem to actually get used in the final render
			//else if (lplf->lfWeight == 700) // this is what shows up rendered in both the UI and name tags
		}
		else if (lplf->lfHeight == 24) // Font 6 - seems to be used for YOU HAVE BEEN DISCONNECTED too
		{
			// weight 700
			lplf->lfHeight = 32;
		}
#ifdef NAMEPLATE_HACK
		else if (lplf->lfHeight == 69)
		{
			lplf->lfHeight = 80;
			lplf->lfWeight = 400;
		}
#endif
	}

	return CreateFontIndirectA(lplf);
}

int __cdecl FH_CInvSlotWnd_CXWnd__DrawColoredRect_Detour(int x1, int y1, int x2, int y2, int color, int clip_x1, int clip_y1, int clip_x2, int clip_y2)
{
	x1 = x2 - 26;
	y1 = y2 - 22;
	//x1 = x1 < 0 ? 0 : x1;
	//y1 = y1 < 0 ? 0 : y1;
	// expand clipping area if needed
	clip_x1 = x1 < clip_x1 ? x1 : clip_x1;
	clip_y1 = y1 < clip_y1 ? y1 : clip_y1;

	//Log("FH_CInvSlotWnd_CXWnd__DrawColoredRect_Detour %d %d %d %d %d %d %d %d %d", x1, y1, x2, y2, color, clip_x1, clip_y1, clip_x2, clip_y2);

	// the original function
	auto CXWnd__DrawColoredRect = (int(__cdecl *)(int, int, int, int, int, int, int, int, int))0x00574380;
	return CXWnd__DrawColoredRect(x1, y1, x2, y2, color, clip_x1, clip_y1, clip_x2, clip_y2);
}

//typedef int(__cdecl *_SaveToUIIniFile)(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpString);
//_SaveToUIIniFile SaveToUIIniFile = (_SaveToUIIniFile)0x004374FA;
typedef char *(*_GetUIIniFilename)();
_GetUIIniFilename GetUIIniFilename = (_GetUIIniFilename)0x00437481;

typedef int *(__cdecl *_GetEQScreenRes)(int *width, int *height);
_GetEQScreenRes GetEQScreenRes = (_GetEQScreenRes)0x004375BD;

char overrideUIIniFileName[200] = { 0 };
typedef int(__cdecl *_LoadStringFromUIIniFile)(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString, DWORD nSize);
_LoadStringFromUIIniFile LoadStringFromUIIniFile_Trampoline;
int __cdecl LoadStringFromUIIniFile_Detour(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString, DWORD nSize)
{
	int ret = 0;
	char m_useIniFile = *(char *)0x0062A78C;

	if (*overrideUIIniFileName)
	{
		char def[50] = { 0 };
		char key[50] = { 0 };
		strncpy(def, lpDefault, 49);
		def[49] = 0;
		strncpy(key, lpKeyName, 49);
		key[49] = 0;
		if (!strcmp(key, "Fades"))
		{
			strcpy(def, "0");
		}
		else if (!strcmp(key, "Alpha") || !strcmp(key, "FadeToAlpha")) // turn off fading
		{
			strcpy(def, "235");
		}
		/*
		else if (!strncmp(key, "BGTint", 6) && strstr(lpAppName, "Chat")) // black tint for chat windows
		{
			strcpy(def, "0");
		}
		*/
		else if (!strcmp(key, "Width") || !strcmp(key, "Height")) // suffix these with the resolution
		{
			int screenwidth, screenheight;
			char buf[50];
			GetEQScreenRes(&screenwidth, &screenheight);
			snprintf(buf, 50, "%dx%d", screenwidth, screenheight);
			strcat(key, buf);
		}

		ret = GetPrivateProfileStringA(lpAppName, key, def, lpReturnedString, nSize, overrideUIIniFileName);
		//Log("LoadStringFromUIIniFile_Detour app:%s key:%s default:%s %d m_useIniFile:%d overrideUIIniFileName %s = %s", lpAppName, lpKeyName, lpDefault, nSize, m_useIniFile, overrideUIIniFileName, lpReturnedString);
	}
	else
	{
		ret = LoadStringFromUIIniFile_Trampoline(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize);
		//Log("LoadStringFromUIIniFile_Detour app:%s key:%s default:%s %d m_useIniFile:%d = %s", lpAppName, lpKeyName, lpDefault, nSize, m_useIniFile, lpReturnedString);
	}


	/*
	if (!strcmp(lpAppName, "RaidWindow") || !strcmp(lpAppName, "HelpWindow"))
	{
		if (!strcmp(lpKeyName, "XPos1920x1080")) strcpy(lpReturnedString, "10");
		if (!strcmp(lpKeyName, "YPos1920x1080")) strcpy(lpReturnedString, "10");
		if (!strcmp(lpKeyName, "Width")) strcpy(lpReturnedString, "300");
		if (!strcmp(lpKeyName, "Height")) strcpy(lpReturnedString, "300");
		if (!strcmp(lpKeyName, "Show")) strcpy(lpReturnedString, "0");
		Log("%s - %s", lpAppName, lpReturnedString);
	}
	*/

	return ret;
}

typedef int(__stdcall *_CSidlScreenWnd__ConvertToRes)(int val, int span, int defaultres, int screenres);
_CSidlScreenWnd__ConvertToRes CSidlScreenWnd__ConvertToRes_Trampoline;
int __stdcall CSidlScreenWnd__ConvertToRes_Detour(int val, int span, int defaultres, int screenres)
{
	int ret = val;

	if (screenres != defaultres)
	{
		// try to scale linearly - this doesn't have great results and is just a fallback for when no layout is defined for a resolution
		ret = val * screenres / defaultres;
		if (ret + span > screenres) ret = screenres - span;
		if (ret < 0) ret = 0;
	}

	//ret = CSidlScreenWnd__ConvertToRes_Trampoline(val, span, defaultres, screenres);
	//Log("CSidlScreenWnd__ConvertToRes_Detour val:%d span:%d defaultres:%d screenres:%d = %d", val, span, defaultres, screenres, ret);

	return ret;
}

void CopyINISection(char *lpAppName, char *sourceFile, char *destFile)
{
	char buf[65536];

	if (GetPrivateProfileSectionA(lpAppName, buf, 65535, sourceFile))
	{
		WritePrivateProfileSectionA(lpAppName, buf, destFile);
	}
}

// trampolines for detoured class methods
class FH;
typedef int(__thiscall *_CDisplay__InitGameUI)(FH *);
_CDisplay__InitGameUI CDisplay__InitGameUI_Trampoline;
typedef void(__thiscall *_CXWnd__GetHitTestRect)(FH *, int *outRect, int type);
_CXWnd__GetHitTestRect CXWnd__GetHitTestRect_Trampoline;

class FH
{
public:
	int CTextureFont__DrawWrappedText_Trampoline(void *text, int x1, int y1, int x2, int y2, int clip_x1, int clip_y1, int clip_x2, int clip_y2, int a10, short a11, int a12)const;
	int CTextureFont__DrawWrappedText_Detour(void *text, int x1, int y1, int x2, int y2, int clip_x1, int clip_y1, int clip_x2, int clip_y2, int a10, short a11, int a12)const
	{
		if (a11 != 17) // this is used for coin on cursor too, but that value is centered - it's 17 when it's coin
		{
			x1 = x2 - 23;
			y1 = y2 - 22;
		}
		//x1 = x1 < 0 ? 0 : x1;
		//y1 = y1 < 0 ? 0 : y1;
		// expand clipping area if needed
		//clip_x1 = x1 < clip_x1 ? x1 : clip_x1;
		//clip_y1 = y1 < clip_y1 ? y1 : clip_y1;

		//Log("CTextureFont__DrawWrappedText_Detour 0x%lx 0x%lx %d %d %d %d %d %d %d %d %d %d %d", this, text, x1, y1, x2, y2, clip_x1, clip_y1, clip_x2, clip_y2, a10, a11, a12);

		return this->CTextureFont__DrawWrappedText_Trampoline(text, x1, y1, x2, y2, clip_x1, clip_y1, clip_x2, clip_y2, a10, a11, a12);
		//auto CXWnd__DrawColoredRect = (int(__cdecl *)(int, int, int, int, int, int, int, int, int))0x00574380;
		//CXWnd__DrawColoredRect(x1, y1, x2, y2, 0xff00ff00, clip_x1, clip_y1, clip_x2, clip_y2);
	}

	/*
	int CSidlScreenWnd__LoadIniInfo_Trampoline();
	int CSidlScreenWnd__LoadIniInfo_Detour()
	{
		char *m_useIniFile = (char *)0x0062A78C;
		char *wndName = (char *)(*(int *)((int)this + 288) + 20);
		char *uiSkinDir = (char *)0x0063D3C0;
		char *uiFileName = GetUIIniFilename();
		int *x2 = (int *)((int)this + 56);
		int *y1 = (int *)((int)this + 52);
		int *x1 = (int *)((int)this + 48);
		int *y2 = (int *)((int)this + 60);
		char defaultsFile[200];
		int ret;
		int *curwidth = (int *)((int)this + 292);
		int *curheight = (int *)((int)this + 296);

		defaultsFile[0] = 0;
		strcpy(defaultsFile, uiSkinDir);
		strcat(defaultsFile, "defaults.ini");

		Log("CSidlScreenWnd__LoadIniInfo_Detour %s m_useIniFile:%d %s %s x1:%d y1:%d x2:%d y2:%d", wndName, *m_useIniFile, uiFileName, defaultsFile, *x1, *y1, *x2, *y2);

		if (!*m_useIniFile) // resetting layout
		{
			if (uiFileName && *uiFileName) // no filename when at char select
			{
				strcpy(overrideUIIniFileName, defaultsFile);
				*m_useIniFile = 1;
				ret = this->CSidlScreenWnd__LoadIniInfo_Trampoline();
				overrideUIIniFileName[0] = 0;
				*m_useIniFile = 0;
			}
			else
			{
				// char select screen
				ret = this->CSidlScreenWnd__LoadIniInfo_Trampoline();
			}
		}
		else // normal ini load
		{
			ret = this->CSidlScreenWnd__LoadIniInfo_Trampoline();
		}

		return ret;
	}
	*/

	//int CDisplay__InitGameUI_Trampoline();
	int CDisplay__InitGameUI_Detour()
	{
		char *m_useIniFile = (char *)0x0062A78C;
		char *uiSkinDir = (char *)0x0063D3C0;
		char *uiFileName = GetUIIniFilename();

		char defaultsFile[300];
		int ret;

		//Log("CDisplay__InitGameUI_Detour - Start m_useIniFile:%d uiSkinDir:%s uiFileName:%s", *m_useIniFile, uiSkinDir, uiFileName);

		if (!*m_useIniFile && uiFileName && *uiFileName) // resetting layout, no filename when at char select
		{
			strcpy(defaultsFile, uiSkinDir);
			strcat(defaultsFile, "defaults.ini");
			strcpy(overrideUIIniFileName, defaultsFile);
			*m_useIniFile = 1;

			//ret = this->CDisplay__InitGameUI_Trampoline();
			ret = CDisplay__InitGameUI_Trampoline(this);

			overrideUIIniFileName[0] = 0;
			*m_useIniFile = 0;

			// bag locations are loaded/saved when the bag is opened so this init call with the override doesn't work for defaulting them
			CopyINISection("BagInv1", defaultsFile, uiFileName);
			CopyINISection("BagInv2", defaultsFile, uiFileName);
			CopyINISection("BagInv3", defaultsFile, uiFileName);
			CopyINISection("BagInv4", defaultsFile, uiFileName);
			CopyINISection("BagInv5", defaultsFile, uiFileName);
			CopyINISection("BagInv6", defaultsFile, uiFileName);
			CopyINISection("BagInv7", defaultsFile, uiFileName);
			CopyINISection("BagInv8", defaultsFile, uiFileName);
			CopyINISection("BagBank1", defaultsFile, uiFileName);
			CopyINISection("BagBank2", defaultsFile, uiFileName);
			CopyINISection("BagBank3", defaultsFile, uiFileName);
			CopyINISection("BagBank4", defaultsFile, uiFileName);
			CopyINISection("BagBank5", defaultsFile, uiFileName);
			CopyINISection("BagBank6", defaultsFile, uiFileName);
			CopyINISection("BagBank7", defaultsFile, uiFileName);
			CopyINISection("BagBank8", defaultsFile, uiFileName);
			CopyINISection("BagWorld1", defaultsFile, uiFileName);
		}
		else
		{
			//ret = this->CDisplay__InitGameUI_Trampoline();
			ret = CDisplay__InitGameUI_Trampoline(this);
		}

		//Log("CDisplay__InitGameUI_Detour - End m_useIniFile:%d uiSkinDir:%s uiFileName:%s", *m_useIniFile, uiSkinDir, uiFileName);

		return ret;
	}

	/*
	char CSidlScreenWnd__StoreIniInfo()
	{
		__asm {mov eax, 0x0056FE10}; \
		__asm {jmp eax}; \
	}
	*/

	void CXWnd__GetHitTestRect_Detour(int *outRect, int type)
	{
		//Log("CXWnd__GetHitTestRect_Detour outRect1(0x%lx) type %d", outRect, type);

		// call real CXWnd__GetHitTestRect
		//(*(void(__thiscall **)(FH *, int *, int))(*(int *)this + 208))(this, outRect, type);
		//(*(void(__thiscall **)(FH *, int *, int))(CXWnd__GetHitTestRect_Trampoline))(this, outRect, type);
		CXWnd__GetHitTestRect_Trampoline(this, outRect, type);

		if (type == 3) // Minimize Box
		{
			outRect[0] += 8; // left
			outRect[1] += 2; // top
			outRect[2] += 8; // right
			outRect[3] += 2; // bottom
		}
		/*
		else if (type == 4) // Tile Box
		{
		}
		*/
		else if (type == 5) // Close Box
		{
			outRect[0] -= 7; // left
			outRect[1] += 1; // top
			outRect[2] -= 7; // right
			outRect[3] += 1; // bottom
		}
	}
};

__declspec(naked) int FH::CTextureFont__DrawWrappedText_Trampoline(void *text, int x1, int y1, int x2, int y2, int clip_x1, int clip_y1, int clip_x2, int clip_y2, int a10, short a11, int a12)const
{
	__asm {mov eax, 0x005A4A30}; \
	__asm {jmp eax}; \
}
//EQ_FUNCTION_AT_ADDRESS(int FH::CTextureFont__DrawWrappedText_Trampoline(void *text, int x1, int y1, int x2, int y2, int clip_x1, int clip_y1, int clip_x2, int clip_y2, int a10, short a11, int a12)const, 0x005A4A30)

/*
__declspec(naked) int FH::CSidlScreenWnd__LoadIniInfo_Trampoline()
{
	__asm {sub esp, 328h}; \
	__asm {mov eax, 0x0056F5A6}; \
	__asm {jmp eax}; \
}
*/

// this looks wrong, like it's missing some of the replaced code?
/*
__declspec(naked) int FH::CDisplay__InitGameUI_Trampoline()
{
	__asm {mov eax, 0x004A60B5 + 5}; \
	__asm {jmp eax}; \
}
*/

/*
int sprintf_Detour_loadskin(char *const Buffer, const char *const format, ...)
{
	va_list ap;

	va_start(ap, format);
	char *cxstr = va_arg(ap, char *);
	cxstr += 20; // get the buffer inside the CXStr
	char useini = va_arg(ap, char);
	va_end(ap);

	return sprintf(Buffer, format, cxstr, useini);
}
*/


/*
_GetEQScreenRes GetEQScreenRes_Trampoline;
int *GetEQScreenRes_Detour(int *width, int *height)
{
	//auto CXWnd__DrawColoredRect = (int(__cdecl *)(int, int, int, int, int, int, int, int, int))0x00574380;
	//auto GetEQScreenRes = (int(__cdecl *)(int *, int *))0x004375BD;
	Log("GetEQScreenRes1 %d %d 0x%lx", *width, *height, GetEQScreenRes_Trampoline);
	GetEQScreenRes_Trampoline(width, height);
	Log("GetEQScreenRes2 %d %d", *width, *height);

	return height;
}
*/

void LoadFontHack()
{
	bool enable = false;

#ifdef INI_FILE
	char buf[2048];
	const char *desc = "This mod doubles the size of the UI.  After entering the game you should do /load defaultx2 - make sure you have uifiles/defaultx2 files properly installed.";
	WritePrivateProfileStringA("ScaledUI", "Description", desc, INI_FILE);
	GetINIString("ScaledUI", "Enabled", "FALSE", buf, sizeof(buf), true);
	enable = ParseINIBool(buf);
#endif

	Log("LoadFontHack(): hack is %s", enable ? "ENABLED" : "DISABLED");

	if (enable)
	{

		// this seems to be only used for the stone UI
		//Detour((void *)0x0055AE55, MakeFont_Detour);

		// EQGfx_Dx8.dll CreateFontA pointer
		//*((int *)(0x000C702C + (int)FH_hEQGfxDll)) = (int)&CreateFontA_Detour;
		// EQGfx_Dx8.dll CreateFontIndirectA pointer
		//*((int *)(0x000C7034 + (int)hEQGfxDll)) = (int)&CreateFontIndirectA_Detour;
		intptr_t addr = (intptr_t)CreateFontIndirectA_Detour;
		Patch((void *)(0x000C7034 + (int)hEQGfxDll), &addr, 4);

		/*
		intptr_t FH_CSidlScreenWnd__LoadIniInfo_Detour;
		{
			auto fp = &FH::CSidlScreenWnd__LoadIniInfo_Detour;
			memcpy(&FH_CSidlScreenWnd__LoadIniInfo_Detour, &fp, 4);
		}
		*/


		// this is the inventory slot bottom right inset label
		// .text:005A79D1 074 E8 AA C9 FC FF                          call    CXWnd__DrawColoredRect
		addr = (intptr_t)FH_CInvSlotWnd_CXWnd__DrawColoredRect_Detour - (intptr_t)0x005A79D6;
		Patch((void *)0x005A79D2, &addr, 4);
		// .text:005A7A30 080 E8 FB CF FF FF                          call    CTextureFont__DrawWrappedText 
		MethodAddressToVariable(FH_CTextureFont__DrawWrappedText_Detour, FH::CTextureFont__DrawWrappedText_Detour);
		addr = FH_CTextureFont__DrawWrappedText_Detour - (intptr_t)0x005A7A35;
		Patch((void *)0x005A7A31, &addr, 4);

		// this is the cursor attachment bottom right inset label
		// .text:0041934B 058 E8 30 B0 15 00                    call    CXWnd__DrawColoredRect
		addr = (intptr_t)FH_CInvSlotWnd_CXWnd__DrawColoredRect_Detour - (intptr_t)0x00419350;
		Patch((void *)0x0041934C, &addr, 4);
		// .text:004192E6 058 E8 95 B0 15 00                    call    CXWnd__DrawColoredRect
		//addr = (intptr_t)FH_CInvSlotWnd_CXWnd__DrawColoredRect_Detour - (intptr_t)0x004192EB;
		//Patch((void *)0x004192E7, &addr, 4); // this is for quantity of coin like platinum
		// .text:0041938C 064 E8 9F B6 18 00                    call    CTextureFont__DrawWrappedText
		addr = FH_CTextureFont__DrawWrappedText_Detour - (intptr_t)0x00419391;
		Patch((void *)0x0041938D, &addr, 4);

		// nop out 10 bytes that set the list item height to 16 instead of using the font height
		// .text:00432804 020 C7 80 40 01 00 00+                mov     dword ptr [eax+140h], 10h
		unsigned char patch[10] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
		Patch((void *)0x00432804, patch, 10); // SkillsWindow
		Patch((void *)0x004323AA, patch, 10); // SkillsSelectWindow

		// CTabWnd tab height
		// .text:005935F4 020 83 C0 08                          add     eax, 8
		char tabHeight = 22;
		Patch((void *)0x005935F6, &tabHeight, 1);

		// Window title text vertical position
		// .text:00572BD3 05C 8D 6C 28 FF                       lea     ebp, [eax + ebp - 1]
		char titleTextOffset = -4;
		Patch((void *)0x00572BD6, &titleTextOffset, 1);

		// CXWnd::GetHitTestRect - this fixes the position of minimize and close buttons on the window titles
		{
			MethodAddressToVariable(FH_CXWnd__GetHitTestRect_Detour, FH::CXWnd__GetHitTestRect_Detour);
			CXWnd__GetHitTestRect_Trampoline = (_CXWnd__GetHitTestRect)DetourWithTrampoline((void *)0x00571540, (void *)FH_CXWnd__GetHitTestRect_Detour, 6);
		}

		// CContainerWnd::SetContainer - this function resizes the window based on the number of container slots
		{
			// Inventory container height padding calculation 36 -> 72
			// .text:00417639 074 83 C0 24                          add     eax, 24h ; '$'
			char containerHeightPadding = 72;
			Patch((void *)0x0041763B, &containerHeightPadding, 1);
			// Inventory container width padding calculation 14 -> 28
			// .text:0041763C 074 83 C1 0E                                      add     ecx, 0Eh
			char containerWidthPadding = 28;
			Patch((void *)0x0041763E, &containerWidthPadding, 1);
		}

		// Tooltips
		{
			// CContainerWnd::PostDraw tooltip vertical offset 22 -> 44
			// .text:00416D7E 028 83 C1 16                                      add     ecx, 16h
			char val = 44;
			Patch((void *)(0x00416D7E + 2), &val, 1);
			// CBazaarWnd::PostDraw
			// .text:004076CA 048 8D 41 16                                      lea     eax, [ecx+16h]
			Patch((void *)(0x004076CA + 2), &val, 1);
			// CBankWnd::PostDraw
			// .text:004049DA 024 83 C1 16                                      add     ecx, 16h
			Patch((void *)(0x004049DA + 2), &val, 1);
			// CGiveWnd::PostDraw
			// .text:0041ED57 024 83 C1 16                                      add     ecx, 16h
			Patch((void *)(0x0041ED57 + 2), &val, 1);
			// CLootWnd::PostDraw
			// .text:004266DA 048 8D 41 16                                      lea     eax, [ecx+16h]
			Patch((void *)(0x004266DA + 2), &val, 1);
			// CMerchantWnd::PostDraw
			// .text:00427191 048 8D 41 16                                      lea     eax, [ecx+16h]
			Patch((void *)(0x00427191 + 2), &val, 1);
			// CTradeWnd::PostDraw
			// .text:0043950C 024 83 C1 16                                      add     ecx, 16h
			Patch((void *)(0x0043950C + 2), &val, 1);
			// CBuffWindow::PostDraw tooltip vertical offset 15 -> 30
			// .text:004096F8 064 6B D2 0F                                      imul    edx, 0Fh
			val = 30;
			Patch((void *)(0x004096F8 + 2), &val, 1);
		}


		// fix UI skin loading dialog in options window - moved this to eqgame.dll
		// .text:00426110 230 E8 57 FE 19 00                    call    _sprintf
		//addr = (intptr_t)sprintf_Detour_loadskin - (intptr_t)0x00426115;
		//Patch((void *)0x00426111, &addr, 4);

		//GetEQScreenRes_Trampoline = (_GetEQScreenRes)DetourWithTrampoline((void *)0x004375BD, GetEQScreenRes_Detour, 24);

		// 
		// default screen size in CSidlScreenWnd::Init is 640x480
		// the layout logic in the client is supposed to scale the default positions of windows but it seems to not work.
		// this hack changes the default size from 640x480 to 7680x4320 which seems to make it work better
		// update: it looks like the default size is only 640x480 because my 3840x2160 size is not one of the supported sizes it has in the switch statement
		// .text:0056E43B 034 C7 86 24 01 00 00 80 02 00 00                 mov     dword ptr [esi+124h], 280h
		// .text:0056E44B 034 C7 86 28 01 00 00 E0 01 00 00                 mov     dword ptr [esi+128h], 1E0h
		// .text:0056E5CC 024 C7 86 24 01 00 00 80 02 00 00                 mov     dword ptr[esi + 124h], 280h
		// .text:0056E5D6 024 C7 86 28 01 00 00 E0 01 00 00                 mov     dword ptr[esi + 128h], 1E0h
		int width = 3840 * 1;
		int height = 2160 * 1;
		//int width = 1920;
		//int height = 1080;
		Patch((void *)(0x0056E43B + 6), &width, 4);
		Patch((void *)(0x0056E44B + 6), &height, 4);
		Patch((void *)(0x0056E5CC + 6), &width, 4);
		Patch((void *)(0x0056E5D6 + 6), &height, 4);

		// these are for the UI layout defaults hack
		LoadStringFromUIIniFile_Trampoline = (_LoadStringFromUIIniFile)DetourWithTrampoline((void *)0x00437542, LoadStringFromUIIniFile_Detour, 8);
		// .text:00437545 004 E8 37 FF FF FF                                call    sub_437481
		intptr_t reladdr = (0x00437481 - (int)LoadStringFromUIIniFile_Trampoline) - 8;
		//Log("LoadStringFromUIIniFile_Trampoline @0x%lx origrel %lx reladdr %lx", LoadStringFromUIIniFile_Trampoline, *(int *)0x00437546, reladdr);
		Patch((void *)((int)LoadStringFromUIIniFile_Trampoline + 4), &reladdr, 4);

		//Detour((void *)0x0056F5A0, (void *)FH_CSidlScreenWnd__LoadIniInfo_Detour);
		// the ConvertToRes function in the client sucks.  this is for scaling the default layout to the current screen size if there isn't a layout for this resolution

		// CDisplay::InitGameUI()
		CSidlScreenWnd__ConvertToRes_Trampoline = (_CSidlScreenWnd__ConvertToRes)DetourWithTrampoline((void *)0x005702A0, (void *)CSidlScreenWnd__ConvertToRes_Detour, 6);
		{
			MethodAddressToVariable(FH_CDisplay__InitGameUI_Detour, FH::CDisplay__InitGameUI_Detour);
			CDisplay__InitGameUI_Trampoline = (_CDisplay__InitGameUI)DetourWithTrampoline((void *)0x004A60B5, (void *)FH_CDisplay__InitGameUI_Detour, 5);
		}
	}
}

#endif

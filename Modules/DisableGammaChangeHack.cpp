#include "eqclientmod.h"
// This hack prevents the game from changing the desktop gamma

#ifdef GAMMA_HACK
#include <stdlib.h>
#include "common.h"
#include "util.h"
#include "settings.h"

typedef int(__cdecl *_t3dSetGammaLevel)(float gamma);
_t3dSetGammaLevel t3dSetGammaLevel_Trampoline;
int __cdecl t3dSetGammaLevel_Detour(float gamma)
{
	return 0;
}

void LoadGammaHack()
{
	bool enable = false;

#ifdef INI_FILE
	char buf[2048];
	const char *desc = "This hack prevents the game from changing the desktop gamma.";
	WritePrivateProfileStringA("DisableGammaChange", "Description", desc, INI_FILE);
	GetINIString("DisableGammaChange", "Enabled", "FALSE", buf, sizeof(buf), true);
	enable = ParseINIBool(buf);
#endif

	Log("LoadGammaHack(): hack is %s", enable ? "ENABLED" : "DISABLED");

	if (enable)
	{
		if (hEQGfxDll)
		{
			// disable gamma change
			// 6C090
			_t3dSetGammaLevel t3dSetGammaLevel = (_t3dSetGammaLevel)((intptr_t)hEQGfxDll + (0x0006C090));
			t3dSetGammaLevel_Trampoline = (_t3dSetGammaLevel)DetourWithTrampoline(t3dSetGammaLevel, t3dSetGammaLevel_Detour, 12);
		}
	}
}


#endif
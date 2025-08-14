#include "eqclientmod.h"

// This adds /hidecorpses LOOTED like what is available in newer clients.

#ifdef HIDECORPSE_HACK
#include "common.h"
#include "util.h"
#include "settings.h"
#include "structs.h"
#include "eq_functions.h"

bool hideCorpsesLooted = false;

typedef void(*_do_hidecorpses)(void *unused, char *args);
_do_hidecorpses do_hidecorpses_Trampoline;
void do_hidecorpses_Detour(void *unused, char *args)
{
	if (!*args)
	{
		char *str13202 = "Please specify what corpses you want to hide: NONE (show all corpses), ALL (hide all corpses), ALLBUTGROUP (hide ALL, except corpses of group members), LOOTED (hide corpses after you loot them).";
		EverQuestObject->dsp_chat(str13202, 273, 1);
		char *str13203 = g_stringTable->getString(13203, 0);
		EverQuestObject->dsp_chat(str13203, 273, 1);
		char *str13204 = g_stringTable->getString(13204, 0);
		EverQuestObject->dsp_chat(str13204, 273, 1);
	}
	else if (!_stricmp(args, "LOOTED"))
	{
		hideCorpsesLooted = !hideCorpsesLooted;
		if (hideCorpsesLooted)
		{
			char *str13303 = "Now any corpse you loot, except your own, will be hidden when you finish looting but leave items on the corpse.";
			EverQuestObject->dsp_chat(str13303, 15, 1);
		}
		else
		{
			// made this one up
			char *msg = "Corpses will no longer be hidden when you finish looting but leave items on the corpse.  Use /hidecorpses NONE to unhide any existing corpses you want to see again.";
			EverQuestObject->dsp_chat(msg, 15, 1);
			//do_hidecorpses_Trampoline(unused, "NONE");
		}
	}
	else
	{
		do_hidecorpses_Trampoline(unused, args);
	}
}

typedef int(__cdecl *_t3dSetActorInvisible)(void *, int);
_t3dSetActorInvisible t3dSetActorInvisible;

class Hidecorpse;
typedef void(__thiscall *_CLootWnd_Deactivate)(Hidecorpse *this_ptr);
_CLootWnd_Deactivate CLootWnd_Deactivate_Trampoline;

class Hidecorpse
{
public:
	void Deactivate_Detour()
	{
		CLootWnd_Deactivate_Trampoline(this);

		if (hideCorpsesLooted)
		{
			EQPlayer *corpse = *(EQPlayer **)0x007F9500;
			if (corpse && corpse->NPC == 2) // NPC corpse
			{
				corpse->LocalInfo->invisible = 1;
				t3dSetActorInvisible((void *)corpse->LocalInfo->actor_ptr, 1);
			}
		}
	}
};

void LoadHidecorpseHack()
{
	bool enable = true;

#ifdef INI_FILE
	char buf[2048];
	const char *desc = "This adds /hidecorpses LOOTED like what is available in newer clients.";
	WritePrivateProfileStringA("HideCorpses", "Description", desc, INI_FILE);
	GetINIString("HideCorpses", "Enabled", "TRUE", buf, sizeof(buf), true);
	enable = ParseINIBool(buf);
#endif

	Log("LoadHidecorpseHack(): hack is %s", enable ? "ENABLED" : "DISABLED");

	if (enable)
	{
		do_hidecorpses_Trampoline = (_do_hidecorpses)DetourWithTrampoline((void *)0x004FB5EB, do_hidecorpses_Detour, 6);

		t3dSetActorInvisible = (_t3dSetActorInvisible)GetProcAddress(hEQGfxDll, "t3dSetActorInvisible");

		MethodAddressToVariable(CLootWnd_Deactivate_Detour, Hidecorpse::Deactivate_Detour);
		CLootWnd_Deactivate_Trampoline = (_CLootWnd_Deactivate)DetourWithTrampoline((void *)0x0042651F, (void *)CLootWnd_Deactivate_Detour, 10);
	}
}

#endif

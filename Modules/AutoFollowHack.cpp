#include "eqclientmod.h"

// tweaks for the built in /follow feature

#ifdef AUTOFOLLOW_HACK
#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "util.h"
#include "settings.h"
#include "eq_functions.h"

float followDistance = 15.0f;

void command_autofollowdistance(void *LocalPlayer, char *text, char *cmd, char *&sep)
{
	char buf[100];
	char *a1 = strtok_s(sep, " ", &sep);

	if (a1 && *a1)
	{
		float newDistance = (float)atof(a1);
		snprintf(buf, 100, "changing follow distance from %0.2f to %0.2f", followDistance, newDistance);
		EverQuestObject->dsp_chat(buf, 281, 0);
		followDistance = newDistance;
	}
	else
	{
		snprintf(buf, 100, "follow distance is %0.2f", followDistance);
		EverQuestObject->dsp_chat(buf, 281, 0);
	}
}

void LoadAutoFollowHack()
{
	bool enable = true;

#ifdef INI_FILE
	char buf[2048];
	const char *desc = "This mod improves /follow reliability.  There is logic in /follow to turn run mode on and off and this actually makes your character crash out of the game if your framerate is high enough.  There is also a smooth turning function to circle around to the followed target which is framerate dependent and causes follow failures.  Both of these things are disabled by this mod.";
	WritePrivateProfileStringA("AutoFollow", "Description", desc, INI_FILE);
	GetINIString("AutoFollow", "Enabled", "TRUE", buf, sizeof(buf), true);
	enable = ParseINIBool(buf);
#endif

	Log("LoadAutoFollowHack(): hack is %s", enable ? "ENABLED" : "DISABLED");

	if (enable)
	{
#ifdef COMMAND_HANDLER
		// follow distance
		// .text:00507D92 118 D8 05 D4 44 5E 00                             fadd    ds:flt_5E44D4
		{
			float *addr = &followDistance;
			Patch((void *)(0x00507D92 + 2), &addr, 4);
		}
#endif

#if false
		// disable run mode being turned off when too close (different method)
		// .text:00507DED 118 88 1D 6D 85 79 00                             mov     run_mode, bl
		{
			uint8_t nop[] = "\x90\x90\x90\x90\x90\x90";
			Patch((void *)0x00507DED, nop, 6);
		}
#endif
		// disable run mode being turned off when too close
		// .text:00507DB0 118 D9 45 FC                                      fld     [ebp+var_4]
		{
			uint8_t x[] = "\xEB\x43\x90";
			Patch((void *)0x00507DB0, x, 3);
		}

		// disable the 'smoothing' where it only turns a little bit at a time if more than a quarter circle off course
		// .text:00507CB1 118 73 0E                                         jnb     short loc_507CC1
		{
			uint8_t nop[] = "\x90\x90";
			Patch((void *)0x00507CB1, &nop, 2);
		}

#ifdef COMMAND_HANDLER
		ChatCommandMap["/autofollowdistance"] = command_autofollowdistance;
#endif
	}
}

#endif

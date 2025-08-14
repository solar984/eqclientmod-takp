#include "eqclientmod.h"

#ifdef FARCLIP_HACK
#include "common.h"
#include "util.h"
#include "settings.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "eq_functions.h"

struct NewZone_Struct
{
	/*0000*/	char	char_name[64];			// Character Name
	/*0064*/	char	zone_short_name[32];	// Zone Short Name
	/*0096*/	char	zone_long_name[278];	// Zone Long Name
	/*0374*/	uint8	ztype;
	/*0375*/	uint8	fog_red[4];				// Red Fog 0-255 repeated over 4 bytes (confirmed)
	/*0379*/	uint8	fog_green[4];			// Green Fog 0-255 repeated over 4 bytes (confirmed)
	/*0383*/	uint8	fog_blue[4];			// Blue Fog 0-255 repeated over 4 bytes (confirmed)
	/*0387*/	uint8	unknown387;
	/*0388*/	float	fog_minclip[4];			// Where the fog begins (lowest clip setting). Repeated over 4 floats. (confirmed)
	/*0404*/	float	fog_maxclip[4];			// Where the fog ends (highest clip setting). Repeated over 4 floats. (confirmed)	
	/*0420*/	float	gravity;
	/*0424*/	uint8	time_type;
	/*0425*/    uint8   rain_chance[4];
	/*0429*/    uint8   rain_duration[4];
	/*0433*/    uint8   snow_chance[4];
	/*0437*/    uint8   snow_duration[4];
	/*0441*/	uint8	specialdates[16];
	/*0457*/	uint8	specialcodes[16];
	/*0473*/	int8	timezone;
	/*0474*/	uint8	sky;					// Sky Type
	/*0475*/	uint8   unknown0475;
	/*0476*/	int16  water_music;
	/*0478*/	int16  normal_music_day;
	/*0480*/	int16  normal_music_night;
	/*0482*/	uint8	unknown0482[2];
	/*0484*/	float	zone_exp_multiplier;	// Experience Multiplier
	/*0488*/	float	safe_y;					// Zone Safe Y
	/*0492*/	float	safe_x;					// Zone Safe X
	/*0496*/	float	safe_z;					// Zone Safe Z
	/*0500*/	float	max_z;					// Guessed
	/*0504*/	float	underworld;				// Underworld, min z (Not Sure?)
	/*0508*/	float	minclip;				// Minimum View Distance
	/*0512*/	float	maxclip;				// Maximum View DIstance
	/*0516*/	uint32	forage_novice;
	/*0520*/	uint32	forage_medium;
	/*0524*/	uint32	forage_advanced;
	/*0528*/	uint32	fishing_novice;
	/*0532*/	uint32	fishing_medium;
	/*0536*/	uint32	fishing_advanced;
	/*0540*/	uint32	skylock;
	/*0544*/	uint16	graveyard_time;
	/*0546*/	uint32	scriptPeriodicHour;
	/*0550*/	uint32	scriptPeriodicMinute;
	/*0554*/	uint32	scriptPeriodicFast;
	/*0558*/	uint32	scriptPlayerDead;
	/*0562*/	uint32	scriptNpcDead;
	/*0566*/	uint32  scriptPlayerEntering;
	/*0570*/	uint16	unknown570;		// ***Placeholder
	/*0572*/
};
struct NewZone_Struct Orig_NewZone;

void NewZone_Intercept(void *dst, const void *src, size_t size)
{
	//char buf[500];

	// save a copy of the original zhdr
	memcpy(&Orig_NewZone, src, size);

	NewZone_Struct *zhdr = (NewZone_Struct *)src;
	//snprintf(buf, 500, "NewZone(%s) '%s' ZEM: %d clip: %0.0f-%0.0f ztype: %d fogcolor: #%02hhX%02hhX%02hhX gravity: %0.2f",
	//	zhdr->zone_short_name, zhdr->zone_long_name, (int)(zhdr->zone_exp_multiplier * 100), zhdr->minclip, zhdr->maxclip, zhdr->ztype,
	//	zhdr->fog_red[0], zhdr->fog_green[0], zhdr->fog_blue[0], zhdr->gravity);
	//EverQuestObject->dsp_chat(buf, 281, 0);

	// increase render distance
	zhdr->maxclip = (float)fmax(zhdr->maxclip, 1000.0) * 2;
	zhdr->fog_maxclip[0] = zhdr->maxclip;
	zhdr->fog_minclip[0] = zhdr->maxclip - 100.0f;
	//snprintf(buf, 500, "farclip %0.0f -> %0.0f", Orig_NewZone.maxclip, zhdr->maxclip);
	//EverQuestObject->dsp_chat(buf, 281, 0);

	memcpy(dst, src, size);
}


// int __thiscall CDisplay::SetFog(int this, int one, float minclip, float maxclip, int red, int green, int blue)
class FarclipHack;
typedef int(__thiscall *_CDisplay__SetFog)(FarclipHack *this_ptr, int one, float minclip, float maxclip, int red, int green, int blue);
_CDisplay__SetFog CDisplay_SetFog_Trampoline;
class FarclipHack
{
public:
	int CDisplay__SetFog_Detour(int one, float minclip, float maxclip, int red, int green, int blue)
	{
		char weatherActive = *(char *)((int)this + 204);
		char processCloud = *(char *)((int)this + 208);
		//char weatherStartStop = *(char *)((int)this + 209);
		//int16 weather_ProcessCloud_counter = *(int16 *)((int)this + 212);
		//int32 weather_ProcessCloudTimer = *(int32 *)((int)this + 232);
		//int32 weather_ProcessCloud_counter2 = *(int32 *)((int)this + 236);

		if (!(weatherActive && processCloud))
		{
			maxclip = *(float *)0x00798984 - 1;
			minclip = maxclip - 100.0f;
		}
		else
		{
			//char buf[200];
			//snprintf(buf, 200, "CDisplay__SetFog_Detour: one %d minclip %f maxclip %f red %u green %u blue %u counter %d PCTimer %d StartStop %d c2 %d", one, minclip, maxclip, (uint8)red, (uint8)green, (uint8)blue, weather_ProcessCloud_counter, weather_ProcessCloudTimer, weatherStartStop, weather_ProcessCloud_counter2);
			//EverQuestObject->dsp_chat(buf, 281, 0);

			// this ends the ProcessCloud sequence on the next cycle
			*(int16 *)((int)this + 212) = 256;
		}
		int ret = CDisplay_SetFog_Trampoline(this, one, minclip, maxclip, red, green, blue);

		//float *farclip = (float *)0x00798984;
		//*(float *)((int)this + 24) = *farclip;

		return ret;
	}
};

void command_farclip(void *LocalPlayer, char *text, char *cmd, char *&sep)
{
	char buf[100];
	float *farclip = (float *)0x00798984;
	char *a1 = strtok_s(sep, " ", &sep);
	if (a1 && *a1)
	{
		float val = (float)atof(a1);
		if (val == 0.0)
		{
			val = Orig_NewZone.maxclip;
			snprintf(buf, 100, "resetting farclip to zone default %0.0f", val);
		}
		else
		{
			snprintf(buf, 100, "changing farclip from %0.0f to %0.0f", *farclip, val);
		}
		EverQuestObject->dsp_chat(buf, 281, 0);
		*farclip = val;
	}
	else
	{
		snprintf(buf, 100, "Usage: /farclip x.x - set far clip distance, 0 to reset to zone default");
		EverQuestObject->dsp_chat(buf, 281, 0);
	}
}

// UseVisActorList
void command_uval(void *LocalPlayer, char *text, char *cmd, char *&sep)
{
	// 00602604 UseVisActorList
	// this enables physics processing for all NPCs not just close visible ones - it has a performance impact and is bad when there are many NPCs in the zone
	int val = *(int *)0x00602604;
	val = val ? 0 : 1;
	*(int *)0x00602604 = val;

	char buf[100];
	snprintf(buf, 100, "UseVisActorList is %s", val ? "ON" : "OFF");
	EverQuestObject->dsp_chat(buf, 281, 0);
}

void LoadFarclipHack()
{
	bool enable = false;
	bool fogHack = false;

#ifdef INI_FILE
	char buf[2048];
	const char *desc = "This increases the draw distance so you can see farther ahead.  This has a performance impact and is really bad in some zones.  "
		"You can use the /farclip command to temporarily change the distance but it's set automatically each time a zone is loaded.  "
		"ExtendFog moves the fog out to the edge of the normal clip plane when underwater and during rain/snow.  This helps visibility and makes the game look better but performance can be bad in some zones.  "
		"The command /uval will toggle a feature called UseVisActorList.  Normally the game doesn't update actors (mobs) that are far away or out of view, so they appear to drop from the sky once you get close enough and they finally get updated positions.  "
		"This toggle will make it so all actors are always updated which looks nicer especially with extended draw distance but it has a performance cost in zones with many actors.  There is also a limit to how far the server sends movement updates so the benefit of this is limited.";
	WritePrivateProfileStringA("Farclip", "Description", desc, INI_FILE);
	GetINIString("Farclip", "Enabled", "FALSE", buf, sizeof(buf), true);
	enable = ParseINIBool(buf);
	GetINIString("Farclip", "ExtendFog", "FALSE", buf, sizeof(buf), true);
	fogHack = ParseINIBool(buf);
#endif

	Log("LoadFarclipHack(): hack is %s", enable ? "ENABLED" : "DISABLED");

	if (enable)
	{
		// .text:004E9100 5F9C E8 4B DB 0D 00                               call    _memcpy
		intptr_t addr = (intptr_t)NewZone_Intercept - (intptr_t)0x004E9100 - 5;
		Patch((void *)(0x004E9100 + 1), &addr, 4);

		// this disables the fog during weather and when underwater
		Log("LoadFarclipHack(): ExtendFog is %s", fogHack ? "ENABLED" : "DISABLED");
		if (fogHack)
		{
			MethodAddressToVariable(CDisplay__SetFog_Detour, FarclipHack::CDisplay__SetFog_Detour);
			CDisplay_SetFog_Trampoline = (_CDisplay__SetFog)DetourWithTrampoline((void *)0x004ADD26, (void *)CDisplay__SetFog_Detour, 6);
		}

#ifdef COMMAND_HANDLER
		ChatCommandMap["/farclip"] = command_farclip;
		ChatCommandMap["/uval"] = command_uval;
#endif
	}
}
#endif

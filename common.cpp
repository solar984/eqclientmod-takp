// the command handler here is wedged in before the normal command handler.
// it doesn't do partial matches like the usual commands

#include "eqclientmod.h"
#include "common.h"
#include "util.h"
#include "structs.h"
#include "settings.h"
#include "eq_functions.h"

// globals
HMODULE hEQGfxDll;
#ifdef COMMAND_HANDLER
std::map<const char *, _slashCommandHandler> ChatCommandMap;
#endif

#ifdef COMMAND_HANDLER
#include <map>

void command_eqclientmod(void *LocalPlayer, char *text, char *cmd, char *&sep)
{
	char buf[200];
	sprintf(buf, "eqclientmod %s solar@heliacal.net", BUILD_VERSION);
	EverQuestObject->dsp_chat(buf, 269, 1);
}

void command_mana(void *LocalPlayer, char *text, char *cmd, char *&sep)
{
	char buf[50];
	intptr_t eqc = *(intptr_t *)0x007F94E8;
	auto EQ_Character__Max_Mana = (short(__fastcall *)(intptr_t)) 0x004B9483;
	auto EQ_Character__Cur_Mana = (short(__fastcall *)(intptr_t)) 0x004B9450;
	snprintf(buf, 50, "mana: %d/%d", EQ_Character__Cur_Mana(eqc), EQ_Character__Max_Mana(eqc));
	EverQuestObject->dsp_chat(buf, 281, 0);
}

void command_autoinventory(void *LocalPlayer, char *text, char *cmd, char *&sep)
{
	EQ_PC *LocalPC = *(EQ_PC **)0x007F94E8;
	auto AutoInventory = (int(__cdecl *)(EQ_PC * pc, void *item, short zero))0x004F0EEB;
	int eqSoundManager = *(int *)0x0063DEA8;
	auto EqSoundManager__WavePlay = (int(__thiscall *)(int this_ptr, int sound, int a3))0x004D518B;
	if (LocalPC->EQ_Character.held_item) // holding an item
	{
		EqSoundManager__WavePlay(eqSoundManager, 146, 0);
		AutoInventory(LocalPC, (void *)LocalPC->EQ_Character.held_item, 0);
	}
}

void command_buffs(void *LocalPlayer, char *text, char *cmd, char *&sep)
{
	char buf[500];
	intptr_t eqc = *(intptr_t *)0x007F94E8;
	struct SpellBuff_Struct *buffs = (struct SpellBuff_Struct *)((char *)eqc + 0x264);

	for (int i = 0; i < 15; i++)
	{
		struct SpellBuff_Struct *b = &buffs[i];
		snprintf(buf, 500, "buff%d bufftype: %d, level %d, bard_modifier: %d, activated: %d, spellid: %d, duration: %d, counters: %d", i, b->bufftype, b->level, b->bard_modifier, b->activated, b->spellid, b->duration, b->counters);
		EverQuestObject->dsp_chat(buf, b->spellid != 0xFFFF ? 269 : 256, 0);
	}
}

// this __fastcall prototype is equivalent to __thiscall, the second argument just isn't meaningful
// int __fastcall CEverQuest__InterpretCmd_Detour_ExecuteCmd(void *this_ptr, void *edx, void *localplayer, char *text)
typedef int(__thiscall *_CEverQuest__InterpretCmd)(void *this_ptr, void *localplayer, char *text);
_CEverQuest__InterpretCmd CEverQuest__InterpretCmd = (_CEverQuest__InterpretCmd)0x0054572F;
class CEverQuest__InterpretCmd_Detour_type
{
public:
	int CEverQuest__InterpretCmd_Detour(void *LocalPlayer, char *text)
	{
		bool handled = false;
		if (LocalPlayer && text && *text == '/')
		{
			char buf2[201];
			strncpy(buf2, text, 200);
			buf2[200] = 0;

			char *sep = buf2;
			char *cmd = strtok_s(sep, " ", &sep);

			for (auto item : ChatCommandMap)
			{
				if (item.first && !strcmp(cmd, item.first))
				{
					handled = true;
					item.second(LocalPlayer, text, cmd, sep);
				}
			}
		}

		if (handled)
		{
			return CEverQuest__InterpretCmd(this, NULL, NULL);
		}

		return CEverQuest__InterpretCmd(this, LocalPlayer, text);
	}
};
#endif

void LoadCommon()
{
	Log("LoadCommon()");

	hEQGfxDll = LoadLibrary("EQGfx_Dx8.DLL");
	if (!hEQGfxDll)
	{
		Log("LoadCommon(): NULL result from LoadLibrary(\"EQGfx_Dx8.DLL\")");
		return;
	}

#ifdef COMMAND_HANDLER
	bool enableCommandHandler = true;

#ifdef INI_FILE
	char buf[2048];
	const char *desc = "This mod adds the extra command handling that some of the other hacks use but it's not strictly necessary to enable this to use the other hacks.  This also adds a /mana command that will print out your current/max mana.  /autoinventory does the same as clicking an item into the auto inventory area where you equip items on your character.";
	WritePrivateProfileStringA("CommandHandler", "Description", desc, INI_FILE);
	GetINIString("CommandHandler", "Enabled", "TRUE", buf, sizeof(buf), true);
	enableCommandHandler = ParseINIBool(buf);
#endif

	Log("LoadCommon(): CommandHandler hack is %s", enableCommandHandler ? "ENABLED" : "DISABLED");

	//SetProcessAffinityMask(GetCurrentProcess(), 1);
	SetThreadAffinityMask(GetCurrentThread(), 1);

	if (enableCommandHandler)
	{
		MethodAddressToVariable(NP_CEverQuest__InterpretCmd_Detour, CEverQuest__InterpretCmd_Detour_type::CEverQuest__InterpretCmd_Detour);

		//intptr_t addr = (intptr_t)CEverQuest__InterpretCmd_Detour_ExecuteCmd - (intptr_t)0x00542394 - 5;
		// .text:00542394 138 E8 96 33 00 00                                call    CEverQuest__InterpretCmd
		//intptr_t addr = (intptr_t)NP_CEverQuest__InterpretCmd_Detour - (intptr_t)0x00542394 - 5;
		//Patch((void *)(0x00542394 + 1), &addr, 4);

		// .text:005423D5 138 E8 55 33 00 00                                call    CEverQuest__InterpretCmd
		//addr = (intptr_t)NP_CEverQuest__InterpretCmd_Detour - (intptr_t)0x005423D5 - 5;
		//Patch((void *)(0x005423D5 + 1), &addr, 4);

		// CChatWindow::WndNotification - this one is for when text is typed into a chat window's input control
		// .text:00413DF6 578 E8 34 19 13 00                                call    CEverQuest__InterpretCmd
		intptr_t addr = (intptr_t)NP_CEverQuest__InterpretCmd_Detour - (intptr_t)0x00413DF6 - 5;
		Patch((void *)(0x00413DF6 + 1), &addr, 4);

		// EQ_Character::DoPassageOfTime
		// .text:004C33D3 344 E8 57 23 08 00                                call    CEverQuest__InterpretCmd
		addr = (intptr_t)NP_CEverQuest__InterpretCmd_Detour - (intptr_t)0x004C33D3 - 5;
		Patch((void *)(0x004C33D3 + 1), &addr, 4);

		ChatCommandMap["/eqclientmod"] = command_eqclientmod;
		ChatCommandMap["/mana"] = command_mana;
		ChatCommandMap["/autoinventory"] = command_autoinventory;
		ChatCommandMap["/buffs"] = command_buffs;
	}
#endif

	//
}

#include "eqclientmod.h"

// This adds an /autofire command that keeps spamming range attack.
// There is also /attackspeedmod for adding to the ranged delay client side.
// The TAKP server enforces the archery delay and will penalize for shots that are too fast.  This mod is useful for testing that.

#ifdef AUTOFIRE_HACK
#include "common.h"
#include "util.h"
#include "settings.h"
#include "eq_functions.h"

int attack_speed_mod = -20;
int autofire = 0;

int *TargetPlayer = (int *)0x007F94EC;
int LastTarget = 0;
bool DisableOnTargetChange = false;

class AttackSpeed;
typedef int(__thiscall *_EQPlayer__ModifyAttackSpeed)(AttackSpeed *this_ptr, int speed, int quiver);
_EQPlayer__ModifyAttackSpeed EQPlayer__ModifyAttackSpeed_Trampoline;

class AttackSpeed
{
public:
	int ModifyAttackSpeed_Detour(int speed, int quiver)
	{
		int ret = EQPlayer__ModifyAttackSpeed_Trampoline(this, speed, quiver);
		if (quiver)
		{
			ret += attack_speed_mod;
			if (ret < 1) ret = 1;
		}

		return ret;
	}
};

#if 0
void command_asm(void *LocalPlayer, char *text, char *cmd, char *&sep)
{
	char buf[50];
	char *a1 = strtok_s(sep, " ", &sep);
	if (a1 && *a1)
	{
		int mod = atoi(a1);

		snprintf(buf, 50, "setting attack_speed_mod to %d", mod);
		attack_speed_mod = mod;
		EverQuestObject->dsp_chat(buf, 281, 0);
	}
	else
	{
		snprintf(buf, 50, "attack_speed_mod is %d", attack_speed_mod);
		EverQuestObject->dsp_chat(buf, 281, 0);
	}
}
#endif

void RangedAttack()
{
	auto EQPlayer__DoAttack = (int(__thiscall *)(int, int, int, int)) 0x0050A0F8;
	int *LocalPlayer = (int *)0x007F94CC;
	int *TargetPlayer = (int *)0x007F94EC;

	// ranged attack
	*((char *)0x007CD844) = 0; // depress button
	EQPlayer__DoAttack(*LocalPlayer, 11, 0, *TargetPlayer);
}

typedef char(__thiscall *_EverQuest__SetAutoAttack)(int thisptr, char val);
_EverQuest__SetAutoAttack EverQuest__SetAutoAttack_Trampoline;

void set_autofire(int state)
{
	char buf[50];
	autofire = state;
	snprintf(buf, 50, "Auto fire %s.", autofire ? "on" : "off");
	EverQuestObject->dsp_chat(buf, 13, 0);

	if (autofire)
	{
		LastTarget = *TargetPlayer;
		EverQuest__SetAutoAttack_Trampoline(0x798540, 0);
		RangedAttack();
	}
}

char __fastcall EverQuest__SetAutoAttack_Detour(int thisptr, int unused, char val)
{
	if (val && autofire)
	{
		set_autofire(0);
	}

	return EverQuest__SetAutoAttack_Trampoline(thisptr, val);
}

void command_autofire(void *LocalPlayer, char *text, char *cmd, char *&sep)
{
	char *a1 = strtok_s(sep, " ", &sep);

	if (a1 && *a1)
	{
		if (!_stricmp(a1, "on"))
		{
			set_autofire(1);
		}
		else if (!_stricmp(a1, "off"))
		{
			set_autofire(0);
		}
		return;
	}

	set_autofire(autofire ? 0 : 1);
}

int CheckTarget()
{
	if (!autofire) return 0;

	if (!*TargetPlayer || (DisableOnTargetChange && LastTarget != *TargetPlayer))
	{
		set_autofire(0);
		return 0;
	}
	LastTarget = *TargetPlayer;

	return 1;
}

void ResetRangedAttackButton2()
{
	if (autofire && CheckTarget())
	{
		RangedAttack();
	}
}

__declspec(naked) void ResetRangedAttackButton()
{
	// reset button
	*((char *)0x007CD844) = 1;

	ResetRangedAttackButton2();

	__asm {mov eax, 0x004C1E87};
	__asm {jmp eax};
}

typedef void(__thiscall *_EQ_Character__DoPassageOfTime)(int thisptr);
_EQ_Character__DoPassageOfTime EQ_Character__DoPassageOfTime_Trampoline;
void __fastcall EQ_Character__DoPassageOfTime_Detour(int thisptr)
{
	CheckTarget();
	EQ_Character__DoPassageOfTime_Trampoline(thisptr);
}

void LoadAutofireHack()
{
	bool enable = true;

#ifdef INI_FILE
	char buf[2048];
	const char *desc = "This mod adds an /autofire command which spams range attack.  It will turn off itself if the target is dead or cleared.";
	WritePrivateProfileStringA("Autofire", "Description", desc, INI_FILE);
	GetINIString("Autofire", "Enabled", "TRUE", buf, sizeof(buf), true);
	enable = ParseINIBool(buf);
	GetINIString("Autofire", "DisableOnTargetChange", "FALSE", buf, sizeof(buf), true);
	DisableOnTargetChange = ParseINIBool(buf);
#endif

	Log("LoadAutofireHack(): hack is %s", enable ? "ENABLED" : "DISABLED");

	if (enable)
	{
		MethodAddressToVariable(EQPlayer__ModifyAttackSpeed_Detour, AttackSpeed::ModifyAttackSpeed_Detour);
		EQPlayer__ModifyAttackSpeed_Trampoline = (_EQPlayer__ModifyAttackSpeed)DetourWithTrampoline((void *)0x0050A039, (void *)EQPlayer__ModifyAttackSpeed_Detour, 7);
		//ChatCommandMap["/attackspeedmod"] = command_asm;
		ChatCommandMap["/autofire"] = command_autofire;

		// this is where the range attack timer is reset in PassageOfTime
		//.text:004C1E80 33C C6 05 44 D8 7C 00 01                          mov     byte_7CD844, 1
		uint8 code[7] = { 0xE9, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90 };
		intptr_t addr = (intptr_t)ResetRangedAttackButton - (intptr_t)0x004C1E80 - 5;
		*((intptr_t *)(code + 1)) = addr;
		Patch((void *)0x004C1E80, code, 7);

		EverQuest__SetAutoAttack_Trampoline = (_EverQuest__SetAutoAttack)DetourWithTrampoline((void *)0x005493B5, EverQuest__SetAutoAttack_Detour, 10);

		EQ_Character__DoPassageOfTime_Trampoline = (_EQ_Character__DoPassageOfTime)DetourWithTrampoline((void *)0x004C131D, EQ_Character__DoPassageOfTime_Detour, 6);
	}
}
#endif

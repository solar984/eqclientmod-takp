#include "eqclientmod.h"
// this hack is to aid debugging/development of buff stacking and isn't a useful mod for playing

#ifdef BUFFS
#include "common.h"
#include "util.h"
#include "structs.h"
#include "eq_functions.h"

class BuffsHack;
typedef int(__thiscall *_EQ_Character__FindAffectSlot)(BuffsHack *this_ptr, __int16 spell_id, int caster_player, int *result_slotnum, int dontremove);
_EQ_Character__FindAffectSlot EQ_Character__FindAffectSlot_Trampoline;

typedef int16(__thiscall *_EQ_Character__CalcAffectChange)(void *this_ptr, EQ_Spell *spelldata, unsigned __int8 level, unsigned __int8 slot_num, void *buff);
_EQ_Character__CalcAffectChange EQ_Character__CalcAffectChange;

typedef void (*_EQ_Character__RemovePCAffectex)(EQ_PC *pc, SpellBuff_Struct *buff, int tell_server);

// struct SpellBuff_Struct *__thiscall EQ_Character::GetEffect(EQ_Character *this, __int16 slotnum)
typedef struct SpellBuff_Struct *(__fastcall *_EQ_Character__GetEffect)(int thisptr, int unused, int16 slotnum);
_EQ_Character__GetEffect EQ_Character__GetEffect = (_EQ_Character__GetEffect)0x004C465A;

typedef int(__fastcall *_EQ_Character__ProcessAffects)(int thisptr);
_EQ_Character__ProcessAffects EQ_Character__ProcessAffects_Trampoline;
int __fastcall EQ_Character__ProcessAffects_Detour(int thisptr)
{
	char buf[200];
	struct SpellBuff_Struct *buff = EQ_Character__GetEffect(thisptr, 0, 0);

	if (buff->bufftype >= 2)
	{
		sprintf(buf, "EQ_Character__ProcessAffects_Detour 1 buff0 duration %d", buff->duration);
		EverQuestObject->dsp_chat(buf, 269, 1);
	}

	int ret = EQ_Character__ProcessAffects_Trampoline(thisptr);

	if (buff->bufftype >= 2)
	{
		sprintf(buf, "EQ_Character__ProcessAffects_Detour 2 buff0 duration %d", buff->duration);
		EverQuestObject->dsp_chat(buf, 269, 1);
		sprintf(buf, ".", buff->duration);
		EverQuestObject->dsp_chat(buf, 269, 1);
	}

	return ret;
}

class BuffsHack
{
public:
	int EQ_Character__FindAffectSlot_Detour(__int16 spell_id, int caster_player, int *result_slotnum, int dontremove)
	{
		char buf[200];

		snprintf(buf, 200, "FindAffectSlot_Detour1: spell_id %d caster_player 0x%p result_slotnum %d dontremove %d", spell_id, (void *)caster_player, *result_slotnum, dontremove);
		EverQuestObject->dsp_chat(buf, 281, 0);
		int ret = EQ_Character__FindAffectSlot_Trampoline(this, spell_id, caster_player, result_slotnum, dontremove);
		snprintf(buf, 200, "FindAffectSlot_Detour2: spell_id %d caster_player 0x%p result_slotnum %d dontremove %d ret 0x%p", spell_id, (void *)caster_player, *result_slotnum, dontremove, (void *)ret);
		EverQuestObject->dsp_chat(buf, 281, 0);

		return ret;
	}

	int16 EQ_Character__CalcAffectChange_FindAffectSlot_Detour(EQ_Spell *spelldata, unsigned __int8 level, unsigned __int8 slot_num, void *buff)
	{
		char buf[200];

		int16 value = EQ_Character__CalcAffectChange(this, spelldata, level, slot_num, buff);
		snprintf(buf, 200, "CalcAffectChange_FindAffectSlot_Detour: spell %d '%s' level %d slot_num %d value %d", spelldata ? spelldata->Id : 0, spelldata ? spelldata->Name : "?", level, slot_num, value);
		EverQuestObject->dsp_chat(buf, 281, 0);

		return value;
	}
};

void LoadBuffsHack()
{
	Log("LoadBuffsHack()");

	MethodAddressToVariable(EQ_Character__FindAffectSlot_Detour, BuffsHack::EQ_Character__FindAffectSlot_Detour);
	EQ_Character__FindAffectSlot_Trampoline = (_EQ_Character__FindAffectSlot)DetourWithTrampoline((void *)0x004C7A3E, (void *)EQ_Character__FindAffectSlot_Detour, 6);

	EQ_Character__CalcAffectChange = (_EQ_Character__CalcAffectChange)0x004C657D;
	MethodAddressToVariable(EQ_Character__CalcAffectChange_FindAffectSlot_Detour, BuffsHack::EQ_Character__CalcAffectChange_FindAffectSlot_Detour);
	// .text:004C800D 040 E8 6B E5 FF FF                                call    EQ_Character__CalcAffectChange
	intptr_t addr = EQ_Character__CalcAffectChange_FindAffectSlot_Detour - (intptr_t)0x004C800D - 5;
	Patch((void *)(0x004C800D + 1), &addr, 4);
	// .text:004C8028 040 E8 50 E5 FF FF                                call    EQ_Character__CalcAffectChange
	addr = EQ_Character__CalcAffectChange_FindAffectSlot_Detour - (intptr_t)0x004C8028 - 5;
	Patch((void *)(0x004C8028 + 1), &addr, 4);

	EQ_Character__ProcessAffects_Trampoline = (_EQ_Character__ProcessAffects)DetourWithTrampoline((void *)0x004C7448, (void *)EQ_Character__ProcessAffects_Detour, 9);
}

#endif

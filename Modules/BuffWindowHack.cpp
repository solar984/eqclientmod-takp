#include "eqclientmod.h"

#ifdef BUFFWINDOW_HACK
#include "common.h"
#include "util.h"
#include "settings.h"
#include "eq_functions.h"
#include "structs.h"

bool buffinfo_enabled = 1;
void command_buffinfo(void *LocalPlayer, char *text, char *cmd, char *&sep)
{
	buffinfo_enabled = !buffinfo_enabled;

	char buf[100];
	snprintf(buf, 100, "Buff info is %s", buffinfo_enabled ? "ON" : "OFF");
	EverQuestObject->dsp_chat(buf, 281, 0);
}


// sizeof 0x168
typedef struct _EQSPAWNINFO
{
	/* 0x0000 */ BYTE Unknown0000; // always equals 0x03
	/* 0x0001 */ CHAR Name[30]; // [0x1E]
	/* 0x001F */ BYTE Unknown001F[37];
	/* 0x0044 */ DWORD ZoneId; // EQ_ZONE_ID_x
	/* 0x0048 */ FLOAT Y;
	/* 0x004C */ FLOAT X;
	/* 0x0050 */ FLOAT Z;
	/* 0x0054 */ FLOAT Heading; // camera view left/right, yaw
	/* 0x0058 */ FLOAT Unknown0058;
	/* 0x005C */ FLOAT MovementSpeed;
	/* 0x0060 */ FLOAT MovementSpeedY;
	/* 0x0064 */ FLOAT MovementSpeedX;
	/* 0x0068 */ FLOAT MovementSpeedZ;
	/* 0x006C */ FLOAT MovementSpeedHeading;
	/* 0x0070 */ FLOAT Unknown0070;
	/* 0x0074 */ FLOAT Pitch; // camera view up/down
	/* 0x0078 */ struct _EQSPAWNINFO *Prev;
	/* 0x007C */ struct _EQSPAWNINFO *Next;
	/* 0x0080 */ PVOID Unknown0080;
	/* 0x0084 */ struct _EQACTORINFO *ActorInfo;
	/* 0x0088 */ struct _EQCHARINFO *CharInfo;
	/* 0x008C */ FLOAT CameraHeightOffset;
	/* 0x0090 */ FLOAT ModelHeightOffset;
	/* 0x0094 */ WORD SpawnId;
	/* 0x0096 */ WORD PetOwnerSpawnId; // spawn id of the owner of this pet spawn
	/* 0x0098 */ DWORD HpMax;
	/* 0x009C */ DWORD HpCurrent;
	/* 0x00A0 */ WORD GuildId;
	/* 0x00A2 */ BYTE Unknown00A2[6];
	/* 0x00A8 */ BYTE Type; // EQ_SPAWN_TYPE_x
	/* 0x00A9 */ BYTE Class; // EQ_CLASS_x
	/* 0x00AA */ WORD Race;  // EQ_RACE_x
	/* 0x00AC */ BYTE Gender; // EQ_GENDER_x
	/* 0x00AD */ BYTE Level;
	/* 0x00AE */ BYTE IsHidden; // 0 = Visible, 1 = Invisible
	/* 0x00AF */ BYTE IsSneaking; // sneaking or snared ; 0 = Normal Movement Speed, 1 = Slow Movement Speed
	/* 0x00B0 */ BYTE IsPlayerKill; // PVP flagged with red name by Priest of Discord
	/* 0x00B1 */ BYTE StandingState; // EQ_STANDING_STATE_x
	/* 0x00B2 */ BYTE LightType; // EQ_LIGHT_TYPE_x
	/* 0x00B3 */ BYTE Unknown00B3;
	/* 0x00B4 */ WORD EquipmentMaterialType[7]; // EQ_EQUIPMENT_MATERIAL_TYPE_x ; Head,Chest,Arms,Wrist,Hands,Legs,Feet
	/* 0x00C2 */ WORD EquipmentPrimaryItemType; // EQ_EQUIPMENT_ITEM_TYPE_x ; Primary
	/* 0x00C4 */ WORD EquipmentSecondaryItemType; // EQ_EQUIPMENT_ITEM_TYPE_x ; Secondary
	/* 0x00C6 */ WORD Unknown00C6;
	/* 0x00C8 */ BYTE Unknown00C8[36];
	/* 0x00EC */ DWORD Unknown00EC;
	/* 0x00F0 */ FLOAT Height; // model height or size/scale (shrink, grow, etc)
	/* 0x00F4 */ FLOAT Unknown00F4;
	/* 0x00F8 */ FLOAT Unknown00F8;
	/* 0x00FC */ FLOAT Unknown00FC;
	/* 0x0100 */ FLOAT MovementBackwardSpeedMultiplier; // modifies how fast you move backwards (Default = 0.4600000083)
	/* 0x0104 */ FLOAT MovementForwardSpeedMultiplier; // modifies how fast you move forwards (Default = 0.6999999881)
	/* 0x0108 */ BYTE IsLinkDead; // LD
	/* 0x0109 */ BYTE IsGameMaster; // GM
	/* 0x010A */ WORD LevitationState; // EQ_LEVITATION_STATE_x
	/* 0x010C */ DWORD TargetType; // EQ_SPAWN_TARGET_TYPE_x
	/* 0x0110 */ DWORD Unknown0110;
	/* 0x0114 */ DWORD AnonymousState; // EQ_ANONYMOUS_STATE_x, /anonymous and /roleplay
	/* 0x0118 */ DWORD Unknown0118;
	/* 0x011C */ DWORD IsAwayFromKeyboard; // AFK
	/* 0x0120 */ BYTE Unknown0120[4];
	/* 0x0124 */ DWORD AlternateAdvancementRank; // AA points title value (0-3) (Venerable, Baroness, etc)
	/* 0x0128 */ BYTE Unknown0128[4];
	/* 0x012C */ CHAR LastName[22]; // surname or title
	/* 0x0142 */ BYTE Unknown0142[10];
	/* 0x014C */ WORD GuildStatus; // guild rank
	/* 0x014E */ WORD Deity; // EQ_DEITY_x
	/* 0x0150 */ BYTE Unknown0150;
	/* 0x0151 */ BYTE Unknown0151[6];
	/* 0x0157 */ BYTE Unknown0157[5];
	/* 0x015C */ DWORD Unknown015C;
	/* 0x0160 */ DWORD Unknown0160;
	/* 0x0164 */ DWORD Unknown0164;
	/* 0x0168 */
} EQSPAWNINFO, *PEQSPAWNINFO;

#define EQ_POINTER_PLAYER_SPAWN_INFO     0x007F94CC
EQSPAWNINFO **EQ_OBJECT_ppPlayerSpawnInfo = (EQSPAWNINFO **)EQ_POINTER_PLAYER_SPAWN_INFO;
#define EQ_OBJECT_PlayerSpawn (*EQ_OBJECT_ppPlayerSpawnInfo)

#define EQ_NUM_BUFFS 15

typedef struct _EQCXRECT
{
	DWORD X1;
	DWORD Y1;
	DWORD X2;
	DWORD Y2;
} EQCXRECT, *PEQCXRECT;

typedef struct _EQCXSTR
{
	/* 0x0000*/ DWORD Font; // 1,6 = Window Title or Button Text, 8 = Hot Button Small Text
	/* 0x0004*/ DWORD MaxLength;
	/* 0x0008*/ DWORD Length;
	/* 0x000C*/ DWORD Encoding; // 0 = ASCII, 1 = Unicode
	/* 0x0010*/ PCRITICAL_SECTION Lock;
	/* 0x0014*/ CHAR Text[1]; // use Length and MaxLength
} EQCXSTR, *PEQCXSTR;

// CXWnd and CSidlScreenWnd share these same properties
// sizeof 0xAC
typedef struct _EQWND
{
	/* 0x0000 */ DWORD Unknown0000; // struct _CSIDLWNDVFTABLE *pvfTable; struct _CXWNDVFTABLE *pvfTable;
	/* 0x0004 */ DWORD MouseHoverTimer;
	/* 0x0008 */ DWORD Unknown0008; // usually equals 2000
	/* 0x000C */ DWORD Unknown000C; // usually equals 500
	/* 0x0010 */ BYTE Unknown0010;
	/* 0x0011 */ BYTE Unknown0011;
	/* 0x0012 */ BYTE IsLocked;
	/* 0x0013 */ BYTE Unknown0013;
	/* 0x0014 */ PVOID Unknown0014;
	/* 0x0018 */ DWORD Unknown0018;
	/* 0x001C */ struct _EQWND *ParentWnd;
	/* 0x0020 */ struct _EQWND *FirstChildWnd;
	/* 0x0024 */ struct _EQWND *NextSiblingWnd;
	/* 0x0028 */ BYTE HasChildren;
	/* 0x0029 */ BYTE HasSiblings;
	/* 0x002A */ BYTE Unknown0030[2];
	/* 0x002C */ DWORD Flags;
	/* 0x0030 */ struct _EQCXRECT Location;
	/* 0x0040 */ struct _EQCXRECT LocationPlaceholder; // used when minimizing the window
	/* 0x0050 */ BYTE IsVisible; // show
	/* 0x0051 */ BYTE IsEnabled;
	/* 0x0052 */ BYTE IsMinimized;
	/* 0x0053 */ BYTE Unknown0053;
	/* 0x0054 */ BYTE IsOpen;
	/* 0x0055 */ BYTE Unknown0055;
	/* 0x0056 */ BYTE IsMouseOver; // mouse is hovering over
	/* 0x0057 */ BYTE Unknown0057;
	/* 0x0058 */ DWORD WindowStyleFlags;
	/* 0x005C */ struct _EQFONT *FontPointer;
	/* 0x0060 */ PEQCXSTR Text;
	/* 0x0064 */ PEQCXSTR ToolTipText;
	/* 0x0068 */ BYTE Unknown0068[28];
	/* 0x0084 */ PEQCXSTR XmlToolTipText;
	/* 0x0088 */ BYTE Unknown0088[22];
	/* 0x009E */ BYTE AlphaTransparency;
	/* 0x009F */ BYTE Unknown009F;
	/* 0x00A0 */ BYTE ZLayer;
	/* 0x00A1 */ BYTE Unknown00A1[7];
	/* 0x00A8 */ DWORD DrawTemplate;
	/* 0x00AC */
} EQWND, *PEQWND;

// the moveable resizable parent windows
// class CSidlScreenWnd
// sizeof 0x138
typedef struct _EQCSIDLWND
{
	/* 0x0000 */ struct _EQWND EQWnd;
	/* 0x00AC */ BYTE Unknown00AC[140]; // skips the rest
	/* 0x0138 */
} EQCSIDLWND, *PEQCSIDLWND;

typedef struct _EQCBUFFWINDOW
{
	/* 0x0000 */ struct _EQCSIDLWND CSidlWnd;
	/* 0x0138 */ BYTE Unknown0138[68];
	/* 0x017C */ struct _EQCSIDLWND *BuffButtonWnd[EQ_NUM_BUFFS]; // CButtonWnd
	/* 0x01B8 */
	/* ...... */
} EQCBUFFWINDOW, *PEQCBUFFWINDOW;


// 21 inventory slots
typedef struct _EQINVENTORY
{
	/* 00 */ struct _EQITEMINFO *EarLeft;
	/* 01 */ struct _EQITEMINFO *Head;
	/* 02 */ struct _EQITEMINFO *Face;
	/* 03 */ struct _EQITEMINFO *EarRight;
	/* 04 */ struct _EQITEMINFO *Neck;
	/* 05 */ struct _EQITEMINFO *Shoulders;
	/* 06 */ struct _EQITEMINFO *Arms;
	/* 07 */ struct _EQITEMINFO *Back;
	/* 08 */ struct _EQITEMINFO *WristLeft;
	/* 09 */ struct _EQITEMINFO *WristRight;
	/* 10 */ struct _EQITEMINFO *Ranged;
	/* 11 */ struct _EQITEMINFO *Hands;
	/* 12 */ struct _EQITEMINFO *Primary;
	/* 13 */ struct _EQITEMINFO *Secondary;
	/* 14 */ struct _EQITEMINFO *RingLeft;
	/* 15 */ struct _EQITEMINFO *RingRight;
	/* 16 */ struct _EQITEMINFO *Chest;
	/* 17 */ struct _EQITEMINFO *Legs;
	/* 18 */ struct _EQITEMINFO *Feet;
	/* 19 */ struct _EQITEMINFO *Waist;
	/* 20 */ struct _EQITEMINFO *Ammo;
} EQINVENTORY, *PEQINVENTORY;

#define EQ_NUM_INVENTORY_SLOTS 21
#define EQ_NUM_INVENTORY_PACK_SLOTS 8
#define EQ_NUM_INVENTORY_BANK_SLOTS 8
#define EQ_NUM_SKILLS 74
#define EQ_NUM_SPELL_BOOK_SPELLS 250 // 32 pages, 8 spells per page, should be 256?
#define EQ_NUM_SPAWNS 8192
#define EQ_NUM_GUILDS 512
#define EQ_NUM_LOOT_WINDOW_ITEMS 30
#define EQ_NUM_HOTBUTTONS 10
#define EQ_NUM_HOTBUTTONS_TOTAL 100
#define EQ_NUM_CONTAINER_SLOTS 10
#define EQ_NUM_SPELL_GEMS 8

// sizeof 0x0A
typedef struct _EQBUFFINFO
{
	/* 0x0000 */ BYTE BuffType;
	/* 0x0001 */ BYTE CasterLevel; // level of player who casted the buff
	/* 0x0002 */ BYTE Modifier; // divide by 10 to get Bard song modifier
	/* 0x0003 */ BYTE Unknown0003;
	/* 0x0004 */ WORD SpellId;
	/* 0x0006 */ WORD Ticks; 
	/* 0x0008 */ WORD Counters;
	/* 0x000A */
} EQBUFFINFO, *PEQBUFFINFO;

// class EQ_Character
typedef struct _EQCHARINFO
{
	/* 0x0000 */ BYTE Unknown0000[2];
	/* 0x0002 */ CHAR Name[64]; // [0x40]
	/* 0x0042 */ CHAR LastName[70]; // [0x46] ; surname or title
	/* 0x0088 */ WORD Gender; // EQ_GENDER_x
	/* 0x008A */ WORD Race; // EQ_RACE_x
	/* 0x008C */ WORD Class; // EQ_CLASS_x
	/* 0x008E */ WORD Unknown008E;
	/* 0x0090 */ WORD Level;
	/* 0x0092 */ WORD Unknown0092;
	/* 0x0094 */ DWORD Experience; // EXP
	/* 0x0098 */ WORD PracticePoints; // Training window
	/* 0x009A */ WORD Mana;
	/* 0x009C */ WORD BaseHP;
	/* 0x009E */ WORD StunnedState; // EQ_STUNNED_STATE_x
	/* 0x00A0 */ WORD BaseSTR;
	/* 0x00A2 */ WORD BaseSTA;
	/* 0x00A4 */ WORD BaseCHA;
	/* 0x00A6 */ WORD BaseDEX;
	/* 0x00A8 */ WORD BaseINT;
	/* 0x00AA */ WORD BaseAGI;
	/* 0x00AC */ WORD BaseWIS;
	/* 0x00AE */ BYTE Unknown00AE[438];
	/* 0x0264 */ struct _EQBUFFINFO Buff[EQ_NUM_BUFFS];
	/* 0x02FA */ BYTE Unknown02FA[1080];
	/* 0x0732 */ WORD SpellBook[EQ_NUM_SPELL_BOOK_SPELLS];
	/* 0x0926 */ BYTE Unknown0926[524];
	/* 0x0B32 */ WORD MemorizedSpell[EQ_NUM_SPELL_GEMS]; // spell gem spell ids
	/* 0x0B42 */ BYTE Unknown0B42[14];
	/* 0x0B50 */ WORD Unknown0B50;
	/* 0x0B52 */ WORD Unknown0B52;
	/* 0x0B54 */ FLOAT ZoneEnterY;
	/* 0x0B58 */ FLOAT ZoneEnterX;
	/* 0x0B5C */ FLOAT ZoneEnterZ;
	/* 0x0B60 */ FLOAT Unknown0060;
	/* 0x0B64 */ BYTE StandingState; // EQ_STANDING_STATE_x
	/* 0x0B65 */ BYTE Unknown0B65[3];
	/* 0x0B68 */ DWORD Platinum;
	/* 0x0B6C */ DWORD Gold;
	/* 0x0B70 */ DWORD Silver;
	/* 0x0B74 */ DWORD Copper;
	/* 0x0B78 */ DWORD BankPlatinum;
	/* 0x0B7C */ DWORD BankGold;
	/* 0x0B80 */ DWORD BankSilver;
	/* 0x0B84 */ DWORD BankCopper;
	/* 0x0B88 */ DWORD CursorPlatinum; // currency held on the mouse cursor
	/* 0x0B8C */ DWORD CursorGold;
	/* 0x0B90 */ DWORD CursorSilver;
	/* 0x0B94 */ DWORD CursorCopper;
	/* 0x0B98 */ BYTE Unknown0B98[16];
	/* 0x0BA8 */ WORD Skill[EQ_NUM_SKILLS];
	/* 0x0C3C */ BYTE Unknown0C3C[64];
	/* 0x0C7C */ WORD Vision1;
	/* 0x0C7E */ BYTE Unknown0C7E[12];
	/* 0x0C8A */ WORD Vision2;
	/* 0x0C8C */ BYTE Unknown0C8C[120];
	/* 0x0D04 */ DWORD IsSwimmingUnderwater;
	/* 0x0D08 */ BYTE Unknown0D08[4];
	/* 0x0D0C */ BYTE Unknown0D0C[4];
	/* 0x0D10 */ BYTE IsAutoSplitEnabled;
	/* 0x0D11 */ BYTE Unknown0D11[43];
	/* 0x0D3C */ DWORD Unknown0D3C;
	/* 0x0D40 */ BYTE Unknown0D40[20];
	/* 0x0D54 */ DWORD Hunger;
	/* 0x0D58 */ DWORD Thirst;
	/* 0x0D5C */ BYTE Unknown0D5C[20];
	/* 0x0D70 */ DWORD ZoneId;
	/* 0x0D74 */ struct _EQSPAWNINFO *SpawnInfo;
	/* 0x0D78 */ struct _EQITEMINFO *CursorItem;
	union
	{
		/* 0x0D7C */ struct _EQINVENTORY Inventory;
		/* 0x0D7C */ struct _EQITEMINFO *InventoryItem[EQ_NUM_INVENTORY_SLOTS];
	};
	/* 0x0DD0 */ struct _EQITEMINFO *InventoryPackItem[EQ_NUM_INVENTORY_PACK_SLOTS];
	/* 0x0DF0 */ BYTE Unknown0DF0[116];
	/* 0x0E64 */ DWORD Unknown0E64;
	/* 0x0E68 */ BYTE Unknown0E68[32];
	/* 0x0E88 */ DWORD Unknown0E88;
	/* 0x0E8C */ BYTE Unknown0E8C[56];
	/* 0x0EC4 */ DWORD ZoneBoundId;
	/* 0x0EC8 */ DWORD Unknown0EC8;
	/* 0x0ECC */ DWORD Unknown0ECC;
	/* 0x0ED0 */ DWORD Unknown0ED0;
	/* 0x0ED4 */ DWORD ZoneBirthId;
	/* 0x0ED8 */ FLOAT ZoneBoundY;
	/* 0x0EDC */ DWORD Unknown0EDC;
	/* 0x0EE0 */ DWORD Unknown0EE0;
	/* 0x0EE4 */ DWORD Unknown0EE4;
	/* 0x0EE8 */ FLOAT ZoneBirthY;
	/* 0x0EEC */ FLOAT ZoneBoundX;
	/* 0x0EF0 */ DWORD Unknown0EF0;
	/* 0x0EF4 */ DWORD Unknown0EF4;
	/* 0x0EF8 */ DWORD Unknown0EF8;
	/* 0x0EFC */ FLOAT ZoneBirthX;
	/* 0x0F00 */ FLOAT ZoneBoundZ;
	/* 0x0F04 */ DWORD Unknown0F04;
	/* 0x0F08 */ DWORD Unknown0F08;
	/* 0x0F0C */ DWORD Unknown0F0C;
	/* 0x0F10 */ FLOAT ZoneBirthZ;
	/* 0x0F14 */ BYTE Unknown0F14[1080];
	/* 0x134C */ WORD Deity;
	/* 0x134E */ WORD GuildId;
	/* 0x1350 */ BYTE Unknown1350[8];
	/* 0x1358 */ BYTE Unknown1358;
	/* 0x1359 */ BYTE Unknown1359;
	/* 0x135A */ BYTE Unknown135A;
	/* 0x135B */ BYTE Unknown135B;
	/* 0x135C */ BYTE Unknown135C;
	/* 0x135D */ BYTE Unknown135D;
	/* 0x135E */ BYTE Stamina; // yellow endurance bar ; 100 = Empty, 0 = Full
	/* 0x135F */ BYTE Unknown135F;
	/* 0x1360 */ BYTE Unknown1360;
	/* 0x1361 */ BYTE AnonymousState;
	/* 0x1362 */ BYTE Unknown1362;
	/* 0x1363 */ BYTE GuildStatus; // guild rank
	/* 0x1364 */ BYTE Drunkness; // 0 = Not Drunk, counts down over time
	/* 0x1365 */ BYTE Unknown1365[451];
	/* 0x1528 */ DWORD AlternateAdvancementExperience;
	/* 0x152C */ BYTE Unknown152C[476];
	/* 0x1708 */ BYTE AirSupply; // air remaining while swimming underwater
	/* 0x1709 */ BYTE Unknown1709[2475];
	/* 0x20B4 */ struct _EQITEMINFO *InventoryBankItem[EQ_NUM_INVENTORY_BANK_SLOTS];
	/* ...... */
} EQCHARINFO, *PEQCHARINFO;

#define EQ_POINTER_EQ_Character 0x007F94E8
#define EQ_POINTER_CHAR_INFO EQ_POINTER_EQ_Character
EQCHARINFO **EQ_OBJECT_ppCharInfo = (EQCHARINFO **)EQ_POINTER_CHAR_INFO;
#define EQ_OBJECT_CharInfo (*EQ_OBJECT_ppCharInfo)
#define EQ_SPELL_ID_NULL 0xFFFF // WORD

void EQ_CalculateTickTime(int ticks, int &hours, int &minutes, int &seconds)
{
	if (ticks > 0)
	{
		seconds = ticks * 6;

		if (seconds > 0)
		{
			hours = seconds / (60 * 60);

			seconds = seconds - hours * (60 * 60);

			if (seconds > 0)
			{
				minutes = seconds / 60;

				seconds = seconds - minutes * 60;
			}
		}
	}
}
void EQ_GetTickTimeString(int ticks, char result[], size_t resultSize)
{
	int hours = 0;
	int minutes = 0;
	int seconds = 0;

	EQ_CalculateTickTime(ticks, hours, minutes, seconds);

	char timeText[128] = { 0 };

	if (hours > 0)
	{
		char hoursText[128];
		_snprintf_s(hoursText, sizeof(hoursText), _TRUNCATE, "%dh", hours);

		strncat_s(timeText, sizeof(timeText), hoursText, _TRUNCATE);
	}

	if (minutes > 0)
	{
		if (hours > 0)
		{
			strncat_s(timeText, sizeof(timeText), " ", _TRUNCATE);
		}

		char minutesText[128];
		_snprintf_s(minutesText, sizeof(minutesText), _TRUNCATE, "%dm", minutes);

		strncat_s(timeText, sizeof(timeText), minutesText, _TRUNCATE);
	}

	if (seconds > 0)
	{
		if (hours > 0 || minutes > 0)
		{
			strncat_s(timeText, sizeof(timeText), " ", _TRUNCATE);
		}

		char secondsText[128];
		_snprintf_s(secondsText, sizeof(secondsText), _TRUNCATE, "%ds", seconds);

		strncat_s(timeText, sizeof(timeText), secondsText, _TRUNCATE);
	}

	strncpy_s(result, resultSize, timeText, _TRUNCATE);
}

void EQ_CXStr_Append(PEQCXSTR *cxstr, PCHAR text)
{
	CXStr *temp = (CXStr *)cxstr;

	(*temp) += text;

	cxstr = (PEQCXSTR *)temp;
}

void EQ_CXStr_Set(PEQCXSTR *cxstr, PCHAR text)
{
	CXStr *temp = (CXStr *)cxstr;

	(*temp) = text;

	cxstr = (PEQCXSTR *)temp;
}

typedef int(__thiscall *_CBuffWindow__RefreshBuffDisplay)(void *this_ptr);
_CBuffWindow__RefreshBuffDisplay EQMACMQ_REAL_CBuffWindow__RefreshBuffDisplay;

int __fastcall EQMACMQ_DETOUR_CBuffWindow__RefreshBuffDisplay(void *this_ptr, void *not_used)
{

	int result = EQMACMQ_REAL_CBuffWindow__RefreshBuffDisplay(this_ptr);
	if (!buffinfo_enabled) return result;

	PEQSPAWNINFO playerSpawn = (PEQSPAWNINFO)EQ_OBJECT_PlayerSpawn;
	if (playerSpawn == NULL)
		return result;

	PEQCBUFFWINDOW buffWindow = (PEQCBUFFWINDOW)this_ptr;

	PEQCHARINFO charInfo = (PEQCHARINFO)EQ_OBJECT_CharInfo;

	if (charInfo == NULL)
	{
		return result;
	}

	for (size_t i = 0; i < EQ_NUM_BUFFS; i++)
	{
		int spellId = charInfo->Buff[i].SpellId;

		if (spellId == EQ_SPELL_ID_NULL)
		{
			continue;
		}

		int buffTicks = charInfo->Buff[i].Ticks;

		if (buffTicks == 0)
		{
			continue;
		}

		char buffTickTimeText[128];
		EQ_GetTickTimeString(buffTicks, buffTickTimeText, sizeof(buffTickTimeText));

		char counterTypeText[128] = "";
		SpellManager *g_spellManager = *(SpellManager **)0x00805CB0;
		EQ_Spell *sp = g_spellManager->spells1[spellId];
#define SE_DiseaseCounter				35
#define SE_PoisonCounter				36
#define SE_CurseCounter					116


		for (int slotnum = 0; slotnum < 12; slotnum++)
		{
			if (sp->Attribute[slotnum] == SE_DiseaseCounter)
			{
				strcpy(counterTypeText, "disease ");
				break;
			}
			if (sp->Attribute[slotnum] == SE_PoisonCounter)
			{
				strcpy(counterTypeText, "poison ");
				break;
			}
			if (sp->Attribute[slotnum] == SE_CurseCounter)
			{
				strcpy(counterTypeText, "curse ");
				break;
			}
		}

		char countersText[128] = "";
		if (charInfo->Buff[i].Counters)
		{
			_snprintf_s(countersText, sizeof(countersText), _TRUNCATE, " (%d %scounter%s)", charInfo->Buff[i].Counters, counterTypeText, charInfo->Buff[i].Counters == 1 ? "" : "s");
		}
		char buffTimeText[128];
		_snprintf_s(buffTimeText, sizeof(buffTimeText), _TRUNCATE, " (%s)%s%s", buffTickTimeText, 
			countersText,
			charInfo->Buff[i].BuffType == 4 ? " (reverse tap)" : "");

		PEQCSIDLWND buffButtonWnd = (PEQCSIDLWND)buffWindow->BuffButtonWnd[i];

		if (buffButtonWnd)
		{
			if (buffButtonWnd->EQWnd.ToolTipText)
			{
				EQ_CXStr_Append(&buffButtonWnd->EQWnd.ToolTipText, buffTimeText);
			}
		}
	}

	return result;
};

void LoadBuffWindowHack()
{
	bool enable = true;

#ifdef INI_FILE
	char buf[2048];
	const char *desc = "This hack adds remaining time and counters to the tooltips in the buff window.";
	WritePrivateProfileStringA("BuffWindow", "Description", desc, INI_FILE);
	GetINIString("BuffWindow", "Enabled", "TRUE", buf, sizeof(buf), true);
	enable = ParseINIBool(buf);
#endif

	Log("LoadBuffWindowHack(): hack is %s", enable ? "ENABLED" : "DISABLED");

	if (enable)
	{
		EQMACMQ_REAL_CBuffWindow__RefreshBuffDisplay = (_CBuffWindow__RefreshBuffDisplay)DetourWithTrampoline((void *)0x00409334, EQMACMQ_DETOUR_CBuffWindow__RefreshBuffDisplay, 6);

#ifdef COMMAND_HANDLER
		ChatCommandMap["/buffinfo"] = command_buffinfo;
#endif
	}
}

#endif

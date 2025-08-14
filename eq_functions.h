#pragma once
#include "eqclientmod.h"

#define EQ_FUNCTION_AT_ADDRESS(function, offset) __declspec(naked) function { __asm{mov eax, offset}; __asm{jmp eax}; }

#define EverQuestObject (*(CEverQuest **)0x00809478)
//#define DisplayObject (*(int **)(0x007F9510)
#define SkillDict ((EQ_Skill **)0x007F7AEC)
#define g_stringTable (*(StringTable **)0x007F9490)

class CEverQuest
{
public:
	void CEverQuest::dsp_chat(const char *text, short color, bool filtered);
	void CEverQuest::dsp_chat(const char *text);
	//	int CEverQuest::InterpretCmd(void *LocalPlayer, char *text);
};

class CXStr
{
public:
	CXStr::~CXStr(void);
	CXStr::CXStr(char const *);
	void CXStr::operator+=(char const *);
	void CXStr::operator=(char const *);
	CXStr &CXStr::operator=(CXStr const &);

	/* 0x0000*/ uint32_t Font; // 1,6 = Window Title or Button Text, 8 = Hot Button Small Text
	/* 0x0004*/ uint32_t MaxLength;
	/* 0x0008*/ uint32_t Length;
	/* 0x000C*/ uint32_t Encoding; // 0 = ASCII, 1 = Unicode
	/* 0x0010*/ PCRITICAL_SECTION Lock;
	/* 0x0014*/ char Text[1]; // use Length and MaxLength
};

class StringTable
{
public:
	char *StringTable::getString(unsigned long ID, bool *result = false);
};

/*
class CLootWnd
{
public:
	void CLootWnd::Deactivate();
	void CLootWnd::RequestLootSlot(int slotIndex, bool autoLoot);
};
*/



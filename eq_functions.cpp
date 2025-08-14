#include "eqclientmod.h"
#include "common.h"
#include "eq_functions.h"

/* CEverQuest */
EQ_FUNCTION_AT_ADDRESS(void CEverQuest::dsp_chat(const char *, short, bool), 0x00537F99);
EQ_FUNCTION_AT_ADDRESS(void CEverQuest::dsp_chat(const char *), 0x005380FD);
//EQ_FUNCTION_AT_ADDRESS(int CEverQuest::InterpretCmd(void *LocalPlayer, char *text), 0x0054572F);

/* CXStr */
EQ_FUNCTION_AT_ADDRESS(CXStr::~CXStr(void), 0x00402247);
EQ_FUNCTION_AT_ADDRESS(CXStr::CXStr(char const *), 0x00575F30);
EQ_FUNCTION_AT_ADDRESS(void CXStr::operator+=(char const *), 0x00577310);
EQ_FUNCTION_AT_ADDRESS(void CXStr::operator=(char const *), 0x00576190);
EQ_FUNCTION_AT_ADDRESS(class CXStr &CXStr::operator=(class CXStr const &), 0x00576140);

/* StringTable */
EQ_FUNCTION_AT_ADDRESS(char *StringTable::getString(unsigned long ID, bool *result), 0x00550EFE);

/* CLootWnd */
//EQ_FUNCTION_AT_ADDRESS(void CLootWnd::Deactivate(), 0x0042651F);
//EQ_FUNCTION_AT_ADDRESS(void CLootWnd::RequestLootSlot(int slotIndex, bool autoLoot), 0x00426B02);


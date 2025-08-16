#include "eqclientmod.h"

#ifdef DEBUG_HACK
#include "common.h"
#include "util.h"
#include <iostream>
#include <chrono>
#include <map>
#include "StackWalker/StackWalker.h"
#include "structs.h"
#include "settings.h"
#include "eq_functions.h"

#include <dbghelp.h>
#include <shellapi.h>
#include <shlobj.h>

//#define WEARCHANGE_DEBUG

bool EnableCrashDump = 1;

HMODULE dbgHelpDll = 0;
typedef BOOL(WINAPI *_MiniDumpWriteDump)(
	HANDLE hProcess,
	DWORD ProcessId,
	HANDLE hFile,
	MINIDUMP_TYPE DumpType,
	PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
	PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	PMINIDUMP_CALLBACK_INFORMATION CallbackParam
	);
_MiniDumpWriteDump MiniDumpWriteDumpPtr = 0;

void GenerateDump(EXCEPTION_POINTERS *pExceptionPointers)
{
	BOOL bMiniDumpSuccessful = 0;
	CHAR szFileName[MAX_PATH] = {};
	HANDLE hDumpFile = 0;
	MINIDUMP_EXCEPTION_INFORMATION ExpParam = {};

	if (!MiniDumpWriteDumpPtr) return;

	snprintf(szFileName, MAX_PATH, "dbg-%u-crash.dmp", GetCurrentProcessId());

	hDumpFile = CreateFileA(szFileName, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

	ExpParam.ThreadId = GetCurrentThreadId();
	ExpParam.ExceptionPointers = pExceptionPointers;
	ExpParam.ClientPointers = TRUE;

	bMiniDumpSuccessful = MiniDumpWriteDumpPtr(GetCurrentProcess(), GetCurrentProcessId(),
		hDumpFile, MiniDumpWithFullMemory, &ExpParam, NULL, NULL);

	CloseHandle(hDumpFile);
}


// the fdebug() function that logs to dbg.txt is at 0x00564B10
typedef int(__cdecl *_fdebug)(char *, ...);
_fdebug fdebug_trampoline;

bool swd_enable = 0;

bool netmsg_enable = false;
bool ring_buf_enable = true;
#define ring_buf_count 256
#define ring_buf_size 1000
unsigned char ring_buf_ix = 0;
char ring_buf[ring_buf_count][ring_buf_size] = {};

void ring_buf_print()
{
	fdebug_trampoline("ring_buf_ix = %u", ring_buf_ix);
	unsigned char ix = ring_buf_ix;
	for (int i = 0; i < ring_buf_count; i++)
	{
		fdebug_trampoline("[%03u] %s", ix, ring_buf[ix]);
		ix++;
	}
}

class MyStackWalker : public StackWalker
{
public:
	const char *label = 0;
	MyStackWalker(const char *in_label = 0) : StackWalker(RetrieveVerbose, 0, GetCurrentProcessId(), GetCurrentProcess())
	{
		if (in_label)
		{
			label = _strdup(in_label);
		}
	}
	virtual ~MyStackWalker()
	{
		if (label)
		{
			free((void *)label);
			label = 0;
		}
	}
protected:
	virtual void OnOutput(LPCSTR szText)
	{
		fdebug_trampoline("StackWalker %s %s", label ? label : "", szText);
	}

	virtual void OnDbgHelpErr(LPCSTR szFuncName, DWORD gle, DWORD64 addr)
	{
		// empty
	}
};

bool LooksStringy(char *p)
{
	if (!p)
		return false;

	for (int i = 0; i < 20; i++)
	{
		if (p[i] > 0 && p[i] <= 127)
			continue;
		if (p[i] == 0 && i > 6)
			return true;
		return false;
	}

	return true;
}

bool BadReadPtr(void *p)
{
	bool bad = true;
	MEMORY_BASIC_INFORMATION mbi = { 0 };

	if (::VirtualQuery(p, &mbi, sizeof(mbi)))
	{
		DWORD mask = (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
		bool b = !(mbi.Protect & mask);
		// check the page is not a guard page
		if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) b = true;

		bad = b;
	}

	if (!bad)
	{
		bad = !LooksStringy((char *)p);
	}

	return bad;
}

void *sub_40231C_trampoline = 0;
void __declspec(naked) sub_40231C_detour()
{
	__asm jmp sub_40231C_trampoline;
}

int __declspec(naked) EQTRACE_detour(char *fmt, ...)
{
	//__asm int 3

	__asm
	{
		push dword ptr[esp + 4];
		call BadReadPtr;
		pop ecx;
		test eax, eax;
		jnz skip;

		jmp fdebug_trampoline;

	skip:
		ret;
	}
}

void debug_start()
{
	time_t seconds;
	seconds = time(NULL);
	fdebug_trampoline("solar debug startup at %d", seconds);

	MyStackWalker sw = MyStackWalker("startup");
	sw.ShowCallstack();

	// detour EQTRACE()
	// sub_40231C detour, stealing space here for detouring EQTRACE()
	sub_40231C_trampoline = DetourWithTrampoline((void *)0x0040231C, (void *)sub_40231C_detour, 14);
	// 2 byte jump to the space we stole from the original function
	unsigned char nearjmp[] = { 0xEB, 11 };
	Patch((void *)0x00402314, nearjmp, 2);
	// detour at the trampoline we jumped to above in the stolen space of sub_402306
	Detour((void *)(0x00402314 + 13), EQTRACE_detour);
	unsigned char nop = 0x90;
	Patch((void *)(0x00402314 + 18), &nop, 1);
	Patch((void *)(0x00402314 + 19), &nop, 1);
}

bool fdebug_ready = 0;
int __declspec(naked) fdebug_detour()
{
	if (!fdebug_ready)
	{
		fdebug_ready = 1;
		debug_start();
	}

	__asm jmp fdebug_trampoline;
}

char stackTraceBuf[5000];
char *stackTraceString(int skipFrames = 2)
{
	void *addrs[10] = {};
	DWORD hash;
	RtlCaptureStackBackTrace(skipFrames, 10, addrs, &hash);

	int len = 0;
	for (size_t i = 0; i < 10; ++i)
	{
		if (addrs[i])
		{
			int count = snprintf(stackTraceBuf + len, 5000 - len, "%p ", addrs[i]);
			if (count >= 0 && count < 5000 - len)
			{
				len += count;
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

	return stackTraceBuf;
}

typedef void(__thiscall *_voidMethod)(void *thisptr);
static _voidMethod CDisplay__StopWorldDisplay_Trampoline;
#if false
static _voidMethod EQPlayer__Destructor_Trampoline;
static _voidMethod EQBBoard__Destructor_Trampoline;
static _voidMethod EQEffect__Destructor_Trampoline;
static _voidMethod EQHSprite__Destructor_Trampoline;
static _voidMethod EQSoundEmitter__Destructor_Trampoline;
static _voidMethod EQItemList__Destructor_Trampoline;
static _voidMethod EQMoneyList__Destructor_Trampoline;
static _voidMethod EQMissile__Destructor_Trampoline;
static _voidMethod EQObject__Destructor_Trampoline;
static _voidMethod EQSwitch__Destructor_Trampoline;
#endif

class DebugHack
{
public:

	void CDisplay__StopWorldDisplay_Detour()
	{
		swd_enable = 1;
		fdebug_trampoline("CDisplay__StopWorldDisplay_Detour enter %p", this);

		//MyStackWalker sw = MyStackWalker("SWD");
		//sw.ShowCallstack();

		if (ring_buf_enable)
			ring_buf_print();

		CDisplay__StopWorldDisplay_Trampoline(this);

		fdebug_trampoline("CDisplay__StopWorldDisplay_Detour leave %p", this);
		swd_enable = 0;
	}

#if false
	void EQPlayer__Destructor_Detour()
	{
		char buf[500] = "[NULL this pointer]";

		EQPlayer **EQPlayerListTop = (EQPlayer **)0x007F9498;

		if (swd_enable && this)
		{
			EQPlayer *thisptr = (EQPlayer *)this;
			char *stack = stackTraceString();
			snprintf(buf, 500, "%p id %d name %s type %d pos %.2f,%.2f,%.2f prev %p next %p top %p stack %s",
				thisptr, thisptr->entityId, thisptr->name ? thisptr->name : "[NULL]", thisptr->type, thisptr->x_pos, thisptr->y_pos, thisptr->z_pos, thisptr->prev, thisptr->next, EQPlayerListTop ? *EQPlayerListTop : 0, stack);
		}

		swd_enable &&fdebug_trampoline("EQPlayer__Destructor_Detour enter (%s)", buf);
		EQPlayer__Destructor_Trampoline(this);
		swd_enable &&fdebug_trampoline("EQPlayer__Destructor_Detour leave (%s) (new top %p)", buf, EQPlayerListTop ? *EQPlayerListTop : 0);
	}

	void EQBBoard__Destructor_Detour()
	{
		char buf[500] = "[NULL this pointer]";

		void **ListTop = (void **)0x007F94B0;

		if (swd_enable && this)
		{
			char *stack = stackTraceString();
			snprintf(buf, 500, "%p top %p stack %s",
				this, ListTop ? *ListTop : 0, stack);
		}

		swd_enable &&fdebug_trampoline("EQBBoard__Destructor_Detour enter (%s)", buf);
		EQBBoard__Destructor_Trampoline(this);
		swd_enable &&fdebug_trampoline("EQBBoard__Destructor_Detour leave (%s) (new top %p)", buf, ListTop ? *ListTop : 0);
	}

	void EQEffect__Destructor_Detour()
	{
		char buf[500] = "[NULL this pointer]";

		void **ListTop = (void **)0x007F94B4;

		if (swd_enable && this)
		{
			char *stack = stackTraceString();
			snprintf(buf, 500, "%p top %p stack %s",
				this, ListTop ? *ListTop : 0, stack);
		}

		swd_enable &&fdebug_trampoline("EQEffect__Destructor_Detour enter (%s)", buf);
		EQEffect__Destructor_Trampoline(this);
		swd_enable &&fdebug_trampoline("EQEffect__Destructor_Detour leave (%s) (new top %p)", buf, ListTop ? *ListTop : 0);
	}

	void EQHSprite__Destructor_Detour()
	{
		char buf[500] = "[NULL this pointer]";

		void **ListTop = (void **)0x007F94A8;

		if (swd_enable && this)
		{
			char *stack = stackTraceString();
			snprintf(buf, 500, "%p top %p stack %s",
				this, ListTop ? *ListTop : 0, stack);
		}

		swd_enable &&fdebug_trampoline("EQHSprite__Destructor_Detour enter (%s)", buf);
		EQHSprite__Destructor_Trampoline(this);
		swd_enable &&fdebug_trampoline("EQHSprite__Destructor_Detour leave (%s) (new top %p)", buf, ListTop ? *ListTop : 0);
	}

	void EQSoundEmitter__Destructor_Detour()
	{
		char buf[500] = "[NULL this pointer]";

		void **ListTop = (void **)0x007F94C0;

		if (swd_enable && this)
		{
			char *stack = stackTraceString();
			snprintf(buf, 500, "%p top %p stack %s",
				this, ListTop ? *ListTop : 0, stack);
		}

		swd_enable &&fdebug_trampoline("EQSoundEmitter__Destructor_Detour enter (%s)", buf);
		EQSoundEmitter__Destructor_Trampoline(this);
		swd_enable &&fdebug_trampoline("EQSoundEmitter__Destructor_Detour leave (%s) (new top %p)", buf, ListTop ? *ListTop : 0);
	}

	void EQItemList__Destructor_Detour()
	{
		char buf[500] = "[NULL this pointer]";

		void **ListTop = (void **)0x007F949C;

		if (swd_enable && this)
		{
			char *stack = stackTraceString();
			snprintf(buf, 500, "%p top %p stack %s",
				this, ListTop ? *ListTop : 0, stack);
		}

		swd_enable &&fdebug_trampoline("EQItemList__Destructor_Detour enter (%s)", buf);
		EQItemList__Destructor_Trampoline(this);
		swd_enable &&fdebug_trampoline("EQItemList__Destructor_Detour leave (%s) (new top %p)", buf, ListTop ? *ListTop : 0);
	}

	void EQMoneyList__Destructor_Detour()
	{
		char buf[500] = "[NULL this pointer]";

		void **ListTop = (void **)0x007F94A0;

		if (swd_enable && this)
		{
			char *stack = stackTraceString();
			snprintf(buf, 500, "%p top %p stack %s",
				this, ListTop ? *ListTop : 0, stack);
		}

		swd_enable &&fdebug_trampoline("EQMoneyList__Destructor_Detour enter (%s)", buf);
		EQMoneyList__Destructor_Trampoline(this);
		swd_enable &&fdebug_trampoline("EQMoneyList__Destructor_Detour leave (%s) (new top %p)", buf, ListTop ? *ListTop : 0);
	}

	void EQMissile__Destructor_Detour()
	{
		char buf[500] = "[NULL this pointer]";

		void **ListTop = (void **)0x007F94A4;

		if (swd_enable && this)
		{
			char *stack = stackTraceString();
			snprintf(buf, 500, "%p top %p stack %s",
				this, ListTop ? *ListTop : 0, stack);
		}

		swd_enable &&fdebug_trampoline("EQMissile__Destructor_Detour enter (%s)", buf);
		EQMissile__Destructor_Trampoline(this);
		swd_enable &&fdebug_trampoline("EQMissile__Destructor_Detour leave (%s) (new top %p)", buf, ListTop ? *ListTop : 0);
	}

	void EQObject__Destructor_Detour()
	{
		char buf[500] = "[NULL this pointer]";

		void **ListTop = (void **)0x007F94AC;

		if (swd_enable && this)
		{
			char *stack = stackTraceString();
			snprintf(buf, 500, "%p top %p stack %s",
				this, ListTop ? *ListTop : 0, stack);
		}

		swd_enable &&fdebug_trampoline("EQObject__Destructor_Detour enter (%s)", buf);
		EQObject__Destructor_Trampoline(this);
		swd_enable &&fdebug_trampoline("EQObject__Destructor_Detour leave (%s) (new top %p)", buf, ListTop ? *ListTop : 0);
	}

	void EQSwitch__Destructor_Detour()
	{
		char buf[500] = "[NULL this pointer]";

		void **ListTop = (void **)0x007F94B8;

		if (swd_enable && this)
		{
			char *stack = stackTraceString();
			snprintf(buf, 500, "%p top %p stack %s",
				this, ListTop ? *ListTop : 0, stack);
		}

		swd_enable &&fdebug_trampoline("EQSwitch__Destructor_Detour enter (%s)", buf);
		EQSwitch__Destructor_Trampoline(this);
		swd_enable &&fdebug_trampoline("EQSwitch__Destructor_Detour leave (%s) (new top %p)", buf, ListTop ? *ListTop : 0);
	}
#endif
};

std::map<unsigned short, char *> opcode_names;
EQPlayer **g_pEQP_IDArray = (EQPlayer **)0x0078C47C;

EQPlayer *getEQPlayerByID(unsigned short id)
{
	EQPlayer *p = g_pEQP_IDArray[id];

	return p;
}

char *getEQPlayerName(unsigned short id)
{
	static char eqpname[100] = {};
	strcpy(eqpname, "(unk entity)");

	EQPlayer *p = getEQPlayerByID(id);
	if (p)
	{
		strcpy(eqpname, p->name);
	}

	return eqpname;
}

SpellManager **g_spellManager = (SpellManager **)0x00805CB0;
char *getSpellName(unsigned short id)
{
	if (id < 4000)
	{
		EQ_Spell *spell_data = (*g_spellManager)->spells1[id];
		return spell_data->Name;
	}

	return "(unk spell)";
}
int frameLast = 0;


char *get_op_name(unsigned short op, char *packet, int len)
{
	static char t[2000] = {};
	unsigned short op2 = (op >> 8) | (op << 8); // byte swap
	char *opname = "";

	if (opcode_names.count(op2) != 0)
	{
		opname = opcode_names.at(op2);
	}
	t[0] = 0;

	if (op == 0x4029) // MSG_EQ_RMPLAYER
	{
		if (len >= 2)
		{
			unsigned short id = *(uint16 *)(packet);
			snprintf(t, 2000, "%s entity_id %hu %s", opname, id, getEQPlayerName(id));
			opname = t;
		}
	}
	else if (op == 0x404a) // MSG_PLAYER_DIED
	{
		if (len >= sizeof(struct Death_Struct))
		{
			struct Death_Struct *s = (struct Death_Struct *)packet;
			snprintf(t, 2000, "%s", opname);
			snprintf(t + strlen(t), 2000 - strlen(t), " entity_id %hu %s", s->spawn_id, getEQPlayerName(s->spawn_id));
			snprintf(t + strlen(t), 2000 - strlen(t), " killer_id %hu %s", s->killer_id, getEQPlayerName(s->killer_id));
			snprintf(t + strlen(t), 2000 - strlen(t), " corpseid %hu %s", s->corpseid, getEQPlayerName(s->corpseid));
			snprintf(t + strlen(t), 2000 - strlen(t), " spell_id %hu %s", s->spell_id, getSpellName((uint16)s->spell_id));
			snprintf(t + strlen(t), 2000 - strlen(t), " attack_skill %hhu", s->spell_id);
			snprintf(t + strlen(t), 2000 - strlen(t), " damage %d", s->damage);
			snprintf(t + strlen(t), 2000 - strlen(t), " | spawn_level %hhu unk007 %hhu unk011 %hhu is_PC %08lx", s->spawn_level, s->unknown007, s->unknonw011, *(int *)&(s->is_PC));
			opname = t;
		}

	}
	else if (op == 0x4046) // MSG_EQ_MISSILEHIT
	{
		if (len >= sizeof(struct Action_Struct))
		{
			struct Action_Struct *s = (struct Action_Struct *)packet;
			snprintf(t, 2000, "%s", opname);
			snprintf(t + strlen(t), 2000 - strlen(t), " target %hu %s", s->target, getEQPlayerName(s->target));
			snprintf(t + strlen(t), 2000 - strlen(t), " source %hu %s", s->source, getEQPlayerName(s->source));
			snprintf(t + strlen(t), 2000 - strlen(t), " level %d", s->level);
			snprintf(t + strlen(t), 2000 - strlen(t), " target_level %d", s->target_level);
			snprintf(t + strlen(t), 2000 - strlen(t), " instrument_mod %d", s->instrument_mod);
			snprintf(t + strlen(t), 2000 - strlen(t), " force %0.2f", s->force);
			snprintf(t + strlen(t), 2000 - strlen(t), " heading %0.2f", s->sequence);
			snprintf(t + strlen(t), 2000 - strlen(t), " pushup_angle %0.2f", s->pushup_angle);
			snprintf(t + strlen(t), 2000 - strlen(t), " type %hhu", s->type);
			snprintf(t + strlen(t), 2000 - strlen(t), " unk25 %hhu", s->unknown25);
			snprintf(t + strlen(t), 2000 - strlen(t), " spell_id_unused %hu", s->spell_id_unused);
			snprintf(t + strlen(t), 2000 - strlen(t), " tap_amount %hd", s->tap_amount);
			snprintf(t + strlen(t), 2000 - strlen(t), " spell %hu %s", s->spell, getSpellName(s->spell));
			snprintf(t + strlen(t), 2000 - strlen(t), " unk32 %hhu", s->unknown32);
			snprintf(t + strlen(t), 2000 - strlen(t), " buff_unknown %hhu", s->buff_unknown);
			snprintf(t + strlen(t), 2000 - strlen(t), " unk34 %hhu", s->unknown34);
			opname = t;
		}
	}
	else if (op == 0x40f3) // MSG_UPDATE_STATS OP_ClientUpdate
	{
		if (len >= sizeof(struct SpawnPositionUpdate_Struct))
		{
			struct SpawnPositionUpdate_Struct *s = (struct SpawnPositionUpdate_Struct *)packet;
			int DisplayObject = *(int *)0x007F9510;
			int frameCurTime = *(int *)(DisplayObject + 200);
			int diff = frameCurTime - frameLast;
			frameLast = frameCurTime;
			snprintf(t, 2000, "%s", opname);
			snprintf(t + strlen(t), 2000 - strlen(t), " frameCurTime %d", frameCurTime);
			snprintf(t + strlen(t), 2000 - strlen(t), " diff %d", diff);
			snprintf(t + strlen(t), 2000 - strlen(t), " entity %hu %s", s->spawn_id, getEQPlayerName(s->spawn_id));
			snprintf(t + strlen(t), 2000 - strlen(t), " anim %hhu heading %hhu delta_heading %hhd pos %hd,%hd,%hd delta_yzx %08x %0.2f,%0.2f,%0.2f", s->anim_type, s->heading, s->delta_heading, s->x_pos, s->y_pos, s->z_pos, s->delta_yzx.value, s->delta_yzx.GetX(), s->delta_yzx.GetY(), s->delta_yzx.GetZ());
			char *stack = stackTraceString(1);
			snprintf(t + strlen(t), 2000 - strlen(t), " stack %s", stack);
			opname = t;
		}
	}

	return opname;
}

typedef int(__cdecl *_send_message)(int conn, int opcode, void *packet, size_t size, int ack_req);
_send_message send_message_trampoline;
int __cdecl send_message_detour(int conn, int opcode, void *packet, size_t size, int ack_req)
{
	// send_message at 0054E51A
	DWORD *fdebug_ix_ptr = (DWORD *)0x0080992C;

	char *opname = get_op_name(opcode, (char *)packet, size);

	if (ring_buf_enable)
	{
		snprintf(ring_buf[ring_buf_ix++], ring_buf_size, "%05u MSG -> %04x len %04d %s", *fdebug_ix_ptr, opcode, size, opname);
	}
	if (netmsg_enable)
	{
		fdebug_trampoline("MSG -> %04x len %04d %s", opcode, size, opname);
	}
#ifdef WEARCHANGE_DEBUG
	if (opcode == 0x4092 && size >= sizeof(struct WearChange_Struct)) // OP_WearChange / MSG_MATERIAL_SWAP
	{
		EQPlayer *LocalPlayer = *(EQPlayer **)0x007F94CC;
		char buf[300];
		struct WearChange_Struct *wc = (struct WearChange_Struct *)packet;
		snprintf(buf, 300, "WC -> spawn_id %d slot %d material %d color %#010x unk03 %d unk06 %d stack %s", wc->spawn_id, wc->wear_slot_id, wc->material, wc->color, wc->unknown03, wc->unknown06, stackTraceString());
		EverQuestObject->dsp_chat(buf, LocalPlayer->entityId == wc->spawn_id ? 269 : 281, 0);
	}
#endif

	return send_message_trampoline(conn, opcode, packet, size, ack_req);
}

typedef DWORD(cdecl *_rdp_receive)(void *g_rdp, int timeout);
_rdp_receive rdp_receive_trampoline = 0;
DWORD rdp_receive_detour(void *g_rdp, int timeout)
{
	int size = 0;
	char *packet = 0;
	WORD opcode = 0;
	DWORD *fdebug_ix_ptr = (DWORD *)0x0080992C;

	// g_rdp is 428 bytes
	DWORD ret = rdp_receive_trampoline(g_rdp, timeout);

	if (ret)
	{
		size = *(int *)(ret + 36);
		if (size > 1)
		{
			packet = (char *)(ret + 60);
			opcode = *(WORD *)packet;
			size -= 2; // subtract opcode header from length
			packet += 2;

			char *opname = get_op_name(opcode, (char *)packet, size);

			if (ring_buf_enable)
			{
				snprintf(ring_buf[ring_buf_ix++], ring_buf_size, "%05u MSG <- %04hx len %04d %s", *fdebug_ix_ptr, opcode, size, opname);
			}
			if (netmsg_enable)
			{
				fdebug_trampoline("MSG <- %04hx len %04d %s", opcode, size, opname);
			}
#ifdef WEARCHANGE_DEBUG
			if (opcode == 0x4092 && size >= sizeof(struct WearChange_Struct)) // OP_WearChange / MSG_MATERIAL_SWAP
			{
				EQPlayer *LocalPlayer = *(EQPlayer **)0x007F94CC;
				char buf[300];
				struct WearChange_Struct *wc = (struct WearChange_Struct *)(packet + 2);
				snprintf(buf, 300, "WC <- spawn_id %d slot %d material %d color %#010x unk03 %d unk06 %d stack %s", wc->spawn_id, wc->wear_slot_id, wc->material, wc->color, wc->unknown03, wc->unknown06, stackTraceString());
				EverQuestObject->dsp_chat(buf, LocalPlayer->entityId == wc->spawn_id ? 259 : 281, 0);
			}
#endif
		}
	}

	return ret;
}

typedef int(__cdecl *_log_exception_dbg)(struct _EXCEPTION_POINTERS *ep, const char *error_string);
_log_exception_dbg log_exception_dbg_trampoline;
int __cdecl log_exception_dbg_detour(struct _EXCEPTION_POINTERS *ep, const char *error_string)
{
	if (ring_buf_enable)
		ring_buf_print();

	MyStackWalker sw = MyStackWalker("EH");
	sw.ShowCallstack(GetCurrentThread(), ep->ContextRecord);

	GenerateDump(ep);

	char *stack = stackTraceString(1);
	fdebug_trampoline("log_exception_dbg_detour stack %s", stack);
	return log_exception_dbg_trampoline(ep, error_string);
}

void LoadDebugHack()
{
	bool enable = false;

#ifdef INI_FILE
	char buf[2048];
	const char *desc = "This hack writes more debugging logs to dbg.txt and preserves them instead of overwriting.  The process id is added to the filename.";
	WritePrivateProfileStringA("Debug", "Description", desc, INI_FILE);
	GetINIString("Debug", "Enabled", "FALSE", buf, sizeof(buf), true);
	enable = ParseINIBool(buf);
	GetINIString("Debug", "CrashDump", "FALSE", buf, sizeof(buf), true);
	EnableCrashDump = ParseINIBool(buf);
	Log("Crash dump file %s", EnableCrashDump ? "Enabled" : "Disabled");
#endif

	Log("LoadDebugHack(): hack is %s", enable ? "ENABLED" : "DISABLED");

	if (enable)
	{
		if (EnableCrashDump)
		{
			HMODULE dbgHelpDll = LoadLibrary("dbghelp.dll");
			if (dbgHelpDll)
			{
				MiniDumpWriteDumpPtr = (_MiniDumpWriteDump)GetProcAddress(dbgHelpDll, "MiniDumpWriteDump");
				if (MiniDumpWriteDumpPtr)
				{
					Log("dbghelp function MiniDumpWriteDump at %p", MiniDumpWriteDumpPtr);
				}
			}
		}

		// convert this to always be false so dbg.txt is preserved and grows unbounded
		// if ( !_stat("dbg.txt", &Stat) && Stat.st_size > 100000 )
		// .text:0055916D 4C4 75 10                                         jnz     short loc_55917F
		// .text:00559176 4C4 7E 07                                         jle     short loc_55917F
		//unsigned char jmp = 0xEB;
		//Patch((void *)0x0055916D, &jmp, 1);
		//Patch((void *)0x00559176, &jmp, 1);

		// detour fdebug()
		fdebug_trampoline = (_fdebug)DetourWithTrampoline((void *)0x00564B10, fdebug_detour, 5);

		// this is inside fdebug, which we're still calling into, but let's patch this so we have a unique filename in case there are multiple clients running from the same directory
		// .text:00564BBD 1040 68 B8 CB 60 00                               push    offset aA_1     ; "a"
		// .text:00564BC2 1044 68 2C 62 62 00                               push    offset aDbgTxt; "dbg.txt"
		// .text:00564BC7 1048 E8 BA 28 06 00                               call    _fopen
		static char dbgfile[1024];
		snprintf(dbgfile, 1024, "dbg-%u.txt", GetCurrentProcessId());
		intptr_t pdbgfile = (intptr_t)dbgfile;
		Patch((void *)(0x00564BC2 + 1), &pdbgfile, 4);

		// detour log_exception_dbg used when catching SEH exceptions
		log_exception_dbg_trampoline = (_log_exception_dbg)DetourWithTrampoline((void *)0x0055895B, (void *)log_exception_dbg_detour, 5);

#if false
		// CDisplay::StopWorldDisplay
		MethodAddressToVariable(CDisplay__StopWorldDisplay_Detour, DebugHack::CDisplay__StopWorldDisplay_Detour);
		CDisplay__StopWorldDisplay_Trampoline = (_voidMethod)DetourWithTrampoline((void *)0x004A91DC, (void *)CDisplay__StopWorldDisplay_Detour, 6);
#endif

		// sent and received packets
		send_message_trampoline = (_send_message)DetourWithTrampoline((void *)0x0054E51A, (void *)send_message_detour, 7);
		rdp_receive_trampoline = (_rdp_receive)DetourWithTrampoline((void *)0x0055DEF0, (void *)rdp_receive_detour, 6);

		// temp movement update hack (affects camping too)
		// .text:004C24B3 33C BB E8 03 00 00                                mov     ebx, 1000
		//int interval = 80;
		//Patch((void *)(0x004C24B3 + 1), &interval, 4);

#if false
	// EQPlayer::Destructor
		MethodAddressToVariable(EQPlayer__Destructor_Detour, DebugHack::EQPlayer__Destructor_Detour);
		EQPlayer__Destructor_Trampoline = (_voidMethod)DetourWithTrampoline((void *)0x0050723D, (void *)EQPlayer__Destructor_Detour, 5);

		// EQBBoard::Destructor
		MethodAddressToVariable(EQBBoard__Destructor_Detour, DebugHack::EQBBoard__Destructor_Detour);
		EQBBoard__Destructor_Trampoline = (_voidMethod)DetourWithTrampoline((void *)0x0049C1C7, (void *)EQBBoard__Destructor_Detour, 5);

		// EQEffect::Destructor
		MethodAddressToVariable(EQEffect__Destructor_Detour, DebugHack::EQEffect__Destructor_Detour);
		EQEffect__Destructor_Trampoline = (_voidMethod)DetourWithTrampoline((void *)0x004DBC0F, (void *)EQEffect__Destructor_Detour, 6);

		// EQHSprite::Destructor
		MethodAddressToVariable(EQHSprite__Destructor_Detour, DebugHack::EQHSprite__Destructor_Detour);
		EQHSprite__Destructor_Trampoline = (_voidMethod)DetourWithTrampoline((void *)0x004EEC13, (void *)EQHSprite__Destructor_Detour, 5);

		// EQSoundEmitter::Destructor
		MethodAddressToVariable(EQSoundEmitter__Destructor_Detour, DebugHack::EQSoundEmitter__Destructor_Detour);
		EQSoundEmitter__Destructor_Trampoline = (_voidMethod)DetourWithTrampoline((void *)0x00520F43, (void *)EQSoundEmitter__Destructor_Detour, 5);

		// EQItemList::Destructor
		MethodAddressToVariable(EQItemList__Destructor_Detour, DebugHack::EQItemList__Destructor_Detour);
		EQItemList__Destructor_Trampoline = (_voidMethod)DetourWithTrampoline((void *)0x004EEE66, (void *)EQItemList__Destructor_Detour, 5);

		// EQMoneyList::Destructor
		MethodAddressToVariable(EQMoneyList__Destructor_Detour, DebugHack::EQMoneyList__Destructor_Detour);
		EQMoneyList__Destructor_Trampoline = (_voidMethod)DetourWithTrampoline((void *)0x005039C6, (void *)EQMoneyList__Destructor_Detour, 5);

		// EQMissile::Destructor
		MethodAddressToVariable(EQMissile__Destructor_Detour, DebugHack::EQMissile__Destructor_Detour);
		EQMissile__Destructor_Trampoline = (_voidMethod)DetourWithTrampoline((void *)0x00502DFA, (void *)EQMissile__Destructor_Detour, 6);

		// EQObject::Destructor
		MethodAddressToVariable(EQObject__Destructor_Detour, DebugHack::EQObject__Destructor_Detour);
		EQObject__Destructor_Trampoline = (_voidMethod)DetourWithTrampoline((void *)0x0050677B, (void *)EQObject__Destructor_Detour, 6);

		// EQSwitch::Destructor
		MethodAddressToVariable(EQSwitch__Destructor_Detour, DebugHack::EQSwitch__Destructor_Detour);
		EQSwitch__Destructor_Trampoline = (_voidMethod)DetourWithTrampoline((void *)0x00521434, (void *)EQSwitch__Destructor_Detour, 6);
#endif

		opcode_names =
		{
		{ 0xbf41, "MSG_ABILITY_CHANGE" },
		{ 0x0710, "MSG_ACCESS_GRANTED" },
		{ 0x2470, "MSG_ACCOUNT_INUSE" },
		{ 0x2c40, "MSG_ADD_ILIST" },
		{ 0xed40, "MSG_ADD_LINK" },
		{ 0x0740, "MSG_ADD_MLIST" },
		{ 0x4170, "MSG_ADD_NAME" },
		{ 0x0520, "MSG_ADD_NPC" },
		{ 0xf640, "MSG_ADD_OBJECT" },
		{ 0x9940, "MSG_ADD_SHORTCUT" },
		{ 0xd241, "MSG_ADD_SOULMARK" },
		{ 0x9540, "MSG_ADD_SWITCH" },
		{ 0x0c20, "MSG_ADD_ZCMD" },
		{ 0x0541, "MSG_ALCHEMY" },
		{ 0x0b10, "MSG_ALIVE" },
		{ 0x4c41, "MSG_ALLOW_SCREENSHOTS" },
		{ 0x1442, "MSG_ALTERNATEADV" },
		{ 0xba41, "MSG_APPLY_POISON" },
		{ 0x3f42, "MSG_ARMY" },
		{ 0x4042, "MSG_ARMY_STRUCTURE" },
		{ 0x0042, "MSG_ASSIST_PLAYER" },
		{ 0x1a40, "MSG_AUCTION" },
		{ 0x4241, "MSG_AUTOSAVE_PC" },
		{ 0x5042, "MSG_AVATAR_BAZAAR" },
		{ 0xfa41, "MSG_AVATAR_CREATE_UTIL" },
		{ 0xf141, "MSG_AVATAR_CRYPTKEY" },
		{ 0xa641, "MSG_AVATAR_DELGUILD" },
		{ 0xb841, "MSG_AVATAR_REMOVE_GUILD" },
		{ 0x5c42, "MSG_AVATAR_TEST_ITEMS" },
		{ 0x9340, "MSG_BANDAGE" },
		{ 0x1142, "MSG_BAZAAR_MANAGE" },
		{ 0xa640, "MSG_BCAST_TEXT" },
		{ 0x8c41, "MSG_BECOMENPC" },
		{ 0x2541, "MSG_BEG" },
		{ 0xe640, "MSG_BEGIN_TRADE" },
		{ 0xaa41, "MSG_BERSERK_OFF" },
		{ 0xa941, "MSG_BERSERK_ON" },
		{ 0x4942, "MSG_BOOT_PC" },
		{ 0xd240, "MSG_BUFF" },
		{ 0x2042, "MSG_BUFF_OTHER_GROUP" },
		{ 0x2442, "MSG_BUY_TRADERITEM" },
		{ 0x3841, "MSG_CALC_EXP" },
		{ 0x0742, "MSG_CAMP" },
		{ 0xd040, "MSG_CANCEL_DUEL" },
		{ 0x5a41, "MSG_CANCEL_SNEAKHIDE" },
		{ 0xdb40, "MSG_CANCEL_TRADE" },
		{ 0x7e41, "MSG_CAST_SPELL" },
		{ 0x9140, "MSG_CHANGE_FORM" },
		{ 0x9541, "MSG_CHANGE_GUILDLEADER" },
		{ 0x5841, "MSG_CHANGE_MEDITATE" },
		{ 0xcb40, "MSG_CHANGE_NAME" },
		{ 0x3241, "MSG_CHANGE_PCAFFECT" },
		{ 0x7140, "MSG_CHANNEL_STATUS" },
		{ 0x1042, "MSG_CHATMSG" },
		{ 0x0980, "MSG_CHAT_ADDRESS" },
		{ 0x2970, "MSG_CHAT_BCAST" },
		{ 0x3b70, "MSG_CHAT_REPOP_ZONE" },
		{ 0x2842, "MSG_CHAT_WORLD_KICK" },
		{ 0x3441, "MSG_CHEATER" },
		{ 0x6d41, "MSG_CHEATER_NOTIFY" },
		{ 0x6942, "MSG_CHECK_COMMAND" },
		{ 0x3c40, "MSG_CHECK_GIVE" },
		{ 0xc841, "MSG_CHECK_ITEMS" },
		{ 0x3742, "MSG_CHECK_ITEMS_ZONE" },
		{ 0xd740, "MSG_CHEST_LOCK" },
		{ 0x4e41, "MSG_CLEAR_FACTIONTABLE" },
		{ 0x0542, "MSG_CLEAR_WORLD_CON" },
		{ 0x2c20, "MSG_CLIENTWASLINKDEAD" },
		{ 0xae40, "MSG_CLIENT_HANDOVER_PC" },
		{ 0x0680, "MSG_CLIENT_RECONNECTING" },
		{ 0xa540, "MSG_CLIENT_SPAWN_NPC" },
		{ 0x2441, "MSG_CLIENT_SPAWN_PCONTROL_NPC" },
		{ 0x2341, "MSG_CMD_INDEX" },
		{ 0xb940, "MSG_COMBINE" },
		{ 0xba40, "MSG_COMBINE_ITEM" },
		{ 0xf641, "MSG_COMP_INITITEMS" },
		{ 0xf741, "MSG_COMP_INITSWITCHES" },
		{ 0x0810, "MSG_CONNECTING" },
		{ 0xb740, "MSG_CONSENT" },
		{ 0xb840, "MSG_CONSENT_LOOT" },
		{ 0xd540, "MSG_CONSENT_VERIFY" },
		{ 0x0210, "MSG_CONTINUE" },
		{ 0x9740, "MSG_CONTINUE_ROUTE" },
		{ 0x2641, "MSG_CONTROL_NPC" },
		{ 0xbe40, "MSG_CONTROL_PLAYER" },
		{ 0xd741, "MSG_CORPSELOG" },
		{ 0x2140, "MSG_CORPSE_XYZ" },
		{ 0x3520, "MSG_CPLAYER_DIED" },
		{ 0x2e20, "MSG_CPLAYER_LOGIN" },
		{ 0x2f20, "MSG_CPLAYER_LOGOUT" },
		{ 0x3020, "MSG_CPLAYER_STATUS" },
		{ 0x2370, "MSG_CREATE_ACCOUNT" },
		{ 0x3f41, "MSG_CREATE_GOLD" },
		{ 0x3e20, "MSG_CREATE_GROUP" },
		{ 0x3d20, "MSG_CREATE_GROUP_RESPONSE" },
		{ 0x2b41, "MSG_CREATE_GUILD" },
		{ 0x5e40, "MSG_CREATE_ITEM" },
		{ 0xe740, "MSG_CREATE_PPOINT" },
		{ 0x5f42, "MSG_CS_RAID" },
		{ 0x8341, "MSG_CURHP" },
		{ 0xff40, "MSG_DEBUG_COMBAT" },
		{ 0xcd40, "MSG_DEBUG_NPC" },
		{ 0xfc41, "MSG_DEBUG_NPC_HATE" },
		{ 0x3e42, "MSG_DEBUG_REQUEST" },
		{ 0x3720, "MSG_DELETE_ALL_CORPSES" },
		{ 0xb441, "MSG_DELETE_BITEM" },
		{ 0xb541, "MSG_DELETE_BMONEY" },
		{ 0xe941, "MSG_DELETE_CORPSE" },
		{ 0x1a41, "MSG_DELETE_GUILD" },
		{ 0xae41, "MSG_DELETE_IITEM" },
		{ 0xb041, "MSG_DELETE_IMONEY" },
		{ 0xe840, "MSG_DELETE_PPOINT" },
		{ 0xef40, "MSG_DELETE_ROUTE" },
		{ 0x4a42, "MSG_DELETE_SPELL" },
		{ 0x8840, "MSG_DELETE_ZCMD" },
		{ 0xa441, "MSG_DELTRACELOGIN" },
		{ 0x2b40, "MSG_DEL_ILIST" },
		{ 0x7d40, "MSG_DEL_INVENTORY" },
		{ 0x0840, "MSG_DEL_MLIST" },
		{ 0x0d41, "MSG" },
		{ 0x5a40, "MSG_DEL_PC" },
		{ 0x2e41, "MSG_DEPOP_NPC" },
		{ 0xb140, "MSG_DEPOP_ZONE" },
		{ 0x6842, "MSG_DEVTUNE_NPC" },
		{ 0xaa40, "MSG_DISARM" },
		{ 0xf341, "MSG_DISARMTRAPS" },
		{ 0xab40, "MSG_DISARM_RESULT" },
		{ 0x9741, "MSG_DISBAND" },
		{ 0xe641, "MSG_DISCIPLINE" },
		{ 0xf241, "MSG_DISCIPLINE_CHANGE" },
		{ 0x0910, "MSG_DISCONNECTING" },
		{ 0x5d42, "MSG_DISPLAY_QUEST_FLAGS" },
		{ 0x6041, "MSG_DOATTACK" },
		{ 0x4070, "MSG_DOES_NAME_EXIST" },
		{ 0x3420, "MSG_DOPLAYERSTATCOUNT" },
		{ 0xdc40, "MSG_DO_TRADE" },
		{ 0xcf40, "MSG_DUEL" },
		{ 0x5e41, "MSG_DUEL_END" },
		{ 0x5d41, "MSG_DUEL_START" },
		{ 0x5f40, "MSG_DUST_CONFIRM" },
		{ 0x4c40, "MSG_DUST_CORPSE" },
		{ 0x0a10, "MSG_ECHO" },
		{ 0x2670, "MSG_ECHOREPLY" },
		{ 0x2570, "MSG_ECHOREQUEST" },
		{ 0xe141, "MSG_EMOTE_WORLD" },
		{ 0xe341, "MSG_EMOTE_ZONE" },
		{ 0xe541, "MSG_ENCRYPTKEY" },
		{ 0xee40, "MSG_END_ROUTE" },
		{ 0x1e40, "MSG_ENVIRON_DMG" },
		{ 0x4540, "MSG_EQ_ADDMISSILE" },
		{ 0x2840, "MSG_EQ_ADDPLAYER" },
		{ 0x4640, "MSG_EQ_MISSILEHIT" },
		{ 0x6b42, "MSG_EQ_NETPLAYER" },
		{ 0x5f41, "MSG_EQ_NETPLAYERBUFF" },
		{ 0x2940, "MSG_EQ_RMPLAYER" },
		{ 0x2a40, "MSG_EQ_UPDATEPLAYER" },
		{ 0x4841, "MSG_EXCEPTION" },
		{ 0x3941, "MSG_EXEFILE_CHECK" },
		{ 0x9041, "MSG_EXPENDITEMCHARGE" },
		{ 0x2941, "MSG_EXPLOST" },
		{ 0x9941, "MSG_EXPUP" },
		{ 0xf541, "MSG_EXTRA_UPDATE_TARGET" },
		{ 0x6840, "MSG_FACTION_NAME" },
		{ 0xac40, "MSG_FEIGNDEATH" },
		{ 0x2440, "MSG_FELLTHRUWORLD" },
		{ 0x2340, "MSG_FINALQUIT" },
		{ 0xd340, "MSG_FINAL_INVENTORY" },
		{ 0x0341, "MSG_FINAL_MONEY" },
		{ 0x6940, "MSG_FIND_PLAYER" },
		{ 0x8f41, "MSG_FISH" },
		{ 0x5641, "MSG_FOOD_CHARGE" },
		{ 0x9440, "MSG_FORAGE" },
		{ 0xb641, "MSG_FORCE_BDELITEM" },
		{ 0xb741, "MSG_FORCE_BDELMONEY" },
		{ 0xb241, "MSG_FORCE_DELITEM" },
		{ 0xb341, "MSG_FORCE_DELMONEY" },
		{ 0x0620, "MSG_FORCE_DUST" },
		{ 0x3842, "MSG_FORCE_TITLE" },
		{ 0xc840, "MSG_FORM_CHANGED" },
		{ 0xd941, "MSG_FOUND_CORPSE" },
		{ 0xbc40, "MSG_FOUND_PLAYER" },
		{ 0xdf41, "MSG_FREEGUILDLOCK" },
		{ 0x6a40, "MSG_FREEZE_PLAYER" },
		{ 0xc541, "MSG_FRIENDS_LIST" },
		{ 0xc641, "MSG_FTELL" },
		{ 0x5741, "MSG_FWATER_UPDATE" },
		{ 0xa541, "MSG_GETGUILDLIST" },
		{ 0x3741, "MSG_GETNPC_REACTION" },
		{ 0xe041, "MSG_GETSTATS" },
		{ 0x3442, "MSG_GET_CORPSE_INFO" },
		{ 0x2f41, "MSG_GET_ITEM_NAMES" },
		{ 0x4f41, "MSG_GET_SAFECOORDS" },
		{ 0xd041, "MSG_GET_SOULMARKS" },
		{ 0x1020, "MSG_GET_VEHICLE_ZONE" },
		{ 0xaf41, "MSG_GIVE_IITEM" },
		{ 0xb141, "MSG_GIVE_IMONEY" },
		{ 0x3e40, "MSG_GIVE_ITEM" },
		{ 0x3f40, "MSG_GIVE_MONEY" },
		{ 0x3d40, "MSG_GIVE_OK" },
		{ 0x2742, "MSG_GMALTGIVE" },
		{ 0x4742, "MSG_GMDEBUG_MESSAGE" },
		{ 0xc141, "MSG_GMEXPGIVE" },
		{ 0xc241, "MSG_GMEXPSET" },
		{ 0xc041, "MSG_GMQUEST" },
		{ 0x3120, "MSG_GMSTATUS" },
		{ 0x3670, "MSG_GMSTATUSNEW" },
		{ 0x1940, "MSG_GOSSIP" },
		{ 0x6e40, "MSG_GOTO_PLAYER" },
		{ 0xd440, "MSG_GRANT_TITLE" },
		{ 0x4340, "MSG_GROUP" },
		{ 0x1740, "MSG_GSAY" },
		{ 0x9241, "MSG_GUILDFILE_DATA" },
		{ 0x9d40, "MSG_GUILDMASTER_GOODBYE" },
		{ 0x0442, "MSG_GUILDMOTD" },
		{ 0x0342, "MSG_GUILDMOTD_SET" },
		{ 0x7b41, "MSG_GUILD_ADDED" },
		{ 0x2741, "MSG_GUILD_EXISTS" },
		{ 0x1e41, "MSG_GUILD_LIST" },
		{ 0x3370, "MSG_GUILD_OWNERSHIP" },
		{ 0x9141, "MSG_GUILD_PEACE" },
		{ 0x1841, "MSG_GUILD_REPLY" },
		{ 0x1f41, "MSG_GUILD_SAY" },
		{ 0x1c41, "MSG_GUILD_STATUS" },
		{ 0x6f41, "MSG_GUILD_WAR" },
		{ 0x0c10, "MSG_HANDLE_IN_USE" },
		{ 0xc940, "MSG_HARMLESS_CLEAR" },
		{ 0x7840, "MSG_HARMLESS_SET" },
		{ 0x0e41, "MSG_HDRS_SENT" },
		{ 0x8641, "MSG_HIDE" },
		{ 0xd441, "MSG_HIDEME" },
		{ 0xb240, "MSG_HITPOINT_UPDATE" },
		{ 0x1241, "MSG_HOUSE_ITEM" },
		{ 0x1041, "MSG_HOUSE_LOCK" },
		{ 0x1141, "MSG_HOUSE_MONEY" },
		{ 0x8440, "MSG_INIT_ENCTABLE" },
		{ 0x6740, "MSG_INIT_FACTION" },
		{ 0x7740, "MSG_INIT_ITLIST" },
		{ 0x7440, "MSG_INIT_TRCLASS" },
		{ 0xb640, "MSG_INSPECT" },
		{ 0x9c41, "MSG_INTIMIDATE" },
		{ 0x0410, "MSG_INVALID_ID" },
		{ 0x0510, "MSG_INVALID_PASSWD" },
		{ 0x4040, "MSG_INVITE" },
		{ 0x1741, "MSG_INVITE_GUILD" },
		{ 0x4240, "MSG_INVITE_OK" },
		{ 0x5c41, "MSG_INVULNERABLE_AVATAR" },
		{ 0xf840, "MSG_ITEM_ENC" },
		{ 0x4642, "MSG_ITEM_FIND" },
		{ 0x2040, "MSG_JUMP" },
		{ 0x1c40, "MSG_KICKFROMWORLD" },
		{ 0x6d40, "MSG_KICK_PLAYER" },
		{ 0x6c40, "MSG_KILL_PLAYER" },
		{ 0xd841, "MSG_KUNARK" },
		{ 0x6e41, "MSG_LAST_NAME" },
		{ 0x8241, "MSG_LAUNCHSPELL_INFO" },
		{ 0x9841, "MSG_LEVELUP" },
		{ 0xf041, "MSG_LFG" },
		{ 0x9a41, "MSG_LIMBOMONEY" },
		{ 0x7941, "MSG_LIMBO_ICON" },
		{ 0x7841, "MSG_LIMBO_IEQ" },
		{ 0x7a41, "MSG_LIMBO_INOTES" },
		{ 0x1d41, "MSG_LIST_GUILD" },
		{ 0x3770, "MSG_LOADAVATAR" },
		{ 0x1240, "MSG_LOAD_AVATARS" },
		{ 0x1340, "MSG_LOAD_CHECKSUMS" },
		{ 0x8240, "MSG_LOAD_ENCTABLE" },
		{ 0x6540, "MSG_LOAD_FACTION" },
		{ 0x7540, "MSG_LOAD_ITLIST" },
		{ 0x6040, "MSG_LOAD_NPC" },
		{ 0x8f40, "MSG_LOAD_TEXTFILE" },
		{ 0x7240, "MSG_LOAD_TRCLASS" },
		{ 0x8540, "MSG_LOAD_ZCMD" },
		{ 0x4242, "MSG_LOCALE" },
		{ 0x0e20, "MSG_LOCATE_VEHICLE" },
		{ 0x5040, "MSG_LOCKED_CORPSE" },
		{ 0x3b40, "MSG_LOCKED_MERCHANT" },
		{ 0x4e40, "MSG_LOCK_CORPSE" },
		{ 0x3940, "MSG_LOCK_MERCHANT" },
		{ 0x5818, "MSG_LOGIN" },
		{ 0x3c70, "MSG_LOGINLIST" },
		{ 0x3f70, "MSG_LOGINSERVER" },
		{ 0xfd40, "MSG_LOGIN_KEYCODE" },
		{ 0x0e10, "MSG_LOGIN_REQUEST" },
		{ 0xcc41, "MSG_LOGME_CHEATER" },
		{ 0x3d70, "MSG_LOGOUTLIST" },
		{ 0x3570, "MSG_LOGOUTSTATUS" },
		{ 0xd940, "MSG_LOGOUT_PLAYER" },
		{ 0xfb41, "MSG_LOGSHOWEQ" },
		{ 0xa040, "MSG_LOOT_SLOT" },
		{ 0x7c41, "MSG_MAKENEW_GUILDLEADER" },
		{ 0x9441, "MSG_MAKE_PEACE" },
		{ 0x9341, "MSG_MAKE_WAR" },
		{ 0x9240, "MSG_MATERIAL_SWAP" },
		{ 0x9d41, "MSG_MEND" },
		{ 0x3540, "MSG_MERCHANT_BUY" },
		{ 0x3840, "MSG_MERCHANT_CLEARSLOT" },
		{ 0x3740, "MSG_MERCHANT_GOODBYE" },
		{ 0x2740, "MSG_MERCHANT_SELL" },
		{ 0x8940, "MSG_MODIFY_ZCMD" },
		{ 0x7b40, "MSG_MOD_INVENTORY" },
		{ 0x6240, "MSG_MOD_ROUTE" },
		{ 0xdd41, "MSG_MOTD" },
		{ 0xe441, "MSG_MOVELOG" },
		{ 0x4741, "MSG_MOVE_CHARGE" },
		{ 0x2c41, "MSG_MOVE_ITEM" },
		{ 0x2d41, "MSG_MOVE_MONEY" },
		{ 0xa440, "MSG_MOVE_PPOINT" },
		{ 0x8c40, "MSG_NAME_APPROVE" },
		{ 0xcc40, "MSG_NAME_CHANGED" },
		{ 0x8b40, "MSG_NAME_SUBMIT" },
		{ 0x4940, "MSG_NEW_PC" },
		{ 0x0741, "MSG_NEW_TEXT" },
		{ 0x1920, "MSG_NEW_TEXT_RESPONSE" },
		{ 0xce40, "MSG_NOTE_TEXT" },
		{ 0xef41, "MSG_NO_NAME_APPROVAL" },
		{ 0x3920, "MSG_NPC_BCAST" },
		{ 0x0820, "MSG_NPC_CORPSE" },
		{ 0xf740, "MSG_NPC_ENC" },
		{ 0x7e40, "MSG_NPC_ITEM" },
		{ 0x7f40, "MSG_NPC_MONEY" },
		{ 0x0242, "MSG_NPC_REPOP_ZONE" },
		{ 0x5e42, "MSG_NPC_SAY_TEXT" },
		{ 0x3a70, "MSG_NUMMONSTERS_INWORLD" },
		{ 0x1f70, "MSG_NUMPLAYERS_INWORLD" },
		{ 0x2070, "MSG_NUMPLAYERS_VALIDATE_ACCOUNTKEY" },
		{ 0xbc41, "MSG_OFFVEHICLE" },
		{ 0xbb41, "MSG_ONVEHICLE" },
		{ 0x1b40, "MSG_OOC" },
		{ 0x3942, "MSG_OVERRIDE_TIMER" },
		{ 0x9a40, "MSG_PARTY_EXPERIENCE" },
		{ 0xbb40, "MSG_PARTY_NAMES" },
		{ 0x0010, "MSG_PASS" },
		{ 0xea40, "MSG_PASS_ITEMS" },
		{ 0x2520, "MSG_PCGUILD_UPDATE" },
		{ 0x2242, "MSG_PC_MONSTER_OK" },
		{ 0x0380, "MSG_PC_RECEIVED" },
		{ 0x5941, "MSG_PC_TRANSFERRED" },
		{ 0xac41, "MSG_PERMAKILL" },
		{ 0x4542, "MSG_PET_COMMAND" },
		{ 0x0141, "MSG_PKILL_CLEAR" },
		{ 0x0041, "MSG_PKILL_SET" },
		{ 0xbf40, "MSG_PLAYER_CONTROLLED" },
		{ 0x4a40, "MSG_PLAYER_DIED" },
		{ 0xbd40, "MSG_PLAYER_FROZEN" },
		{ 0xc440, "MSG_PLAYER_GONETO" },
		{ 0xc340, "MSG_PLAYER_KILLED" },
		{ 0x3620, "MSG_PLAYER_LOGOUT" },
		{ 0xc640, "MSG_PLAYER_SUMMONED" },
		{ 0xc240, "MSG_PLAYER_UNCONTROLLED" },
		{ 0xc040, "MSG_PLAYER_UNFROZEN" },
		{ 0xa140, "MSG_PLAY_ANIM" },
		{ 0x0142, "MSG_PLAY_MUSICTRACK" },
		{ 0x4840, "MSG_PLAY_SOUND" },
		{ 0x0c41, "MSG" },
		{ 0x9e41, "MSG_PQ_CHECKIN" },
		{ 0x8e41, "MSG_PQ_CHECKOUT" },
		{ 0xa041, "MSG_PQ_DELETE" },
		{ 0xa141, "MSG_PQ_LOGTOBUG" },
		{ 0xa241, "MSG_PQ_LOGTOFEED" },
		{ 0xa341, "MSG_PQ_LOGTOGUIDE" },
		{ 0x9f41, "MSG_PQ_UNDO_CHECKOUT" },
		{ 0x0f40, "MSG_PQ_UPDATE" },
		{ 0x0641, "MSG_PRESERVE_CORPSE" },
		{ 0x6141, "MSG_PRIMARY_TOGGLE" },
		{ 0x2540, "MSG_PROHIBITEXES" },
		{ 0x2b20, "MSG_PUTCPLAYERINZONE" },
		{ 0xee41, "MSG_QUERY_EXP" },
		{ 0x8140, "MSG_QUEST_ITEM" },
		{ 0xcf41, "MSG_QUEST_PKILL" },
		{ 0x8040, "MSG_QUEST_REWARD" },
		{ 0x5041, "MSG_QUIT_GAME" },
		{ 0xe741, "MSG_RANDOMNUM" },
		{ 0x1640, "MSG_RANDOM_RETURN" },
		{ 0x6341, "MSG_RDPSTAT" },
		{ 0xd840, "MSG_READY_ENTER_WORLD" },
		{ 0xda40, "MSG_READY_TRADE" },
		{ 0xeb40, "MSG_REC_ITEMS" },
		{ 0xd640, "MSG_REFUSE_TRADE" },
		{ 0x8141, "MSG_REJECT_ADDPLAYER" },
		{ 0x1d40, "MSG_REJECT_PC" },
		{ 0x4441, "MSG_RELEASE_GM" },
		{ 0x4541, "MSG_RELEASE_LOOT" },
		{ 0x4641, "MSG_RELEASE_MERCHANT" },
		{ 0x0880, "MSG_RELEASE_PLAYER_AFTER_TIMEOUT" },
		{ 0x1b20, "MSG_RELOAD_GUILDFILE" },
		{ 0x1c20, "MSG_REMOVE_CHEATER" },
		{ 0x1941, "MSG_REMOVE_GUILD" },
		{ 0x4270, "MSG_REMOVE_NAME" },
		{ 0x8a40, "MSG_REMOVE_ZCMD" },
		{ 0xb941, "MSG_RENAME_GUILD" },
		{ 0xf040, "MSG_REPOP_PPOINTS" },
		{ 0x8740, "MSG_REPOP_ZCMD" },
		{ 0xbd41, "MSG_REPORT_TEXT" },
		{ 0x5a42, "MSG_REPORT_TEXT_RAW" },
		{ 0x1420, "MSG_REQCHESTITEMWAFFECT" },
		{ 0x0d20, "MSG_REQITEMWAFFECT" },
		{ 0x4941, "MSG_REQUEST_ITEM" },
		{ 0x1140, "MSG_REQUEST_PETITIONS" },
		{ 0xfe41, "MSG_REQUEST_TARGET" },
		{ 0x0842, "MSG_REQUEST_ZONE" },
		{ 0x1720, "MSG_REQ_AVATAR" },
		{ 0x0920, "MSG_REQ_CORPSEITEM" },
		{ 0x5140, "MSG_REQ_CORPSEITEMS" },
		{ 0x1220, "MSG_REQ_FACTIONTABLE" },
		{ 0x9c40, "MSG_REQ_GUILDMASTER" },
		{ 0x2841, "MSG_REQ_GUILDNAME" },
		{ 0x1b41, "MSG_REQ_GUILD_STATUS" },
		{ 0x0841, "MSG_REQ_HDR" },
		{ 0x0f41, "MSG_REQ_HOUSELOCK" },
		{ 0xb540, "MSG_REQ_INSPECT" },
		{ 0x0320, "MSG_REQ_ITEM" },
		{ 0xe940, "MSG_REQ_ITEMS" },
		{ 0x1541, "MSG_REQ_KEYNUMBER" },
		{ 0x1441, "MSG_REQ_LOOTERS" },
		{ 0x0b40, "MSG_REQ_MERCHANTITEMS" },
		{ 0x0941, "MSG" },
		{ 0x0120, "MSG_REQ_NPC" },
		{ 0x0720, "MSG_REQ_PLAYERS" },
		{ 0x4b40, "MSG_REQ_REPOP" },
		{ 0x2041, "MSG_REQ_SPELLCAST" },
		{ 0x1641, "MSG_REQ_SWITCHNAME" },
		{ 0x5940, "MSG_REQ_THETIME" },
		{ 0x3041, "MSG_REQ_TIME_PLAYED" },
		{ 0xd140, "MSG_REQ_TRADE" },
		{ 0x1642, "MSG_REQ_TRADERITEMS" },
		{ 0x2642, "MSG_REQ_VERSION" },
		{ 0xf440, "MSG_REQ_WHO" },
		{ 0x6442, "MSG_REQUEST_INSPECT_ITEM" },
		{ 0x3b41, "MSG_RESCUE" },
		{ 0x4141, "MSG_RESEND_ADDPLAYER" },
		{ 0x3d42, "MSG_RESET_ACTIVATED_SKILL" },
		{ 0x6a42, "MSG_RESET_MODULATION_TIMER" },
		{ 0xf941, "MSG_RESET_PMONEY" },
		{ 0x3a42, "MSG_RESTORE_FACTION" },
		{ 0xc941, "MSG_RESTORE_PC" },
		{ 0x2a41, "MSG_RESURRECT" },
		{ 0xec41, "MSG_RESURRECT_COMPLETE" },
		{ 0xeb41, "MSG_RESURRECT_PENDING" },
		{ 0x2240, "MSG_RESURRECT_REJECT" },
		{ 0x9b41, "MSG_RESURRECT_RESPONSE" },
		{ 0xf940, "MSG_RETURN_CHEST_ITEMS" },
		{ 0x9b40, "MSG_RM_SWITCH" },
		{ 0xc341, "MSG_RPSERVER" },
		{ 0x1f40, "MSG_RUN" },
		{ 0x3a41, "MSG_RUNSPELL_CHECK" },
		{ 0xea41, "MSG_SACRIFICE" },
		{ 0xab41, "MSG_SAFE_FALL" },
		{ 0x5341, "MSG_SAVEDEADPC" },
		{ 0x5441, "MSG_SAVEREPOP_PC" },
		{ 0x5541, "MSG_SAVEZONE_PC" },
		{ 0x6440, "MSG_SAVE_CON" },
		{ 0x8340, "MSG_SAVE_ENCTABLE" },
		{ 0x6340, "MSG_SAVE_EQ" },
		{ 0x6640, "MSG_SAVE_FACTION" },
		{ 0x1320, "MSG_SAVE_FACTIONTABLE" },
		{ 0x7640, "MSG_SAVE_ITLIST" },
		{ 0x9840, "MSG_SAVE_NOTE" },
		{ 0x6140, "MSG_SAVE_NPC" },
		{ 0x2e40, "MSG_SAVE_PC" },
		{ 0xf140, "MSG_SAVE_ROUTES" },
		{ 0xd141, "MSG_SAVE_SOULMARKS" },
		{ 0x9040, "MSG_SAVE_TEXTFILE" },
		{ 0x7340, "MSG_SAVE_TRCLASS" },
		{ 0x8640, "MSG_SAVE_ZCMD" },
		{ 0x6042, "MSG_SC_RAID" },
		{ 0x5b42, "MSG_SCRIPT_COMMAND" },
		{ 0xa741, "MSG_SEARCH_CORPSE" },
		{ 0x5141, "MSG_SECONDARY_TOGGLE" },
		{ 0x0180, "MSG_SELECT_CHARACTER" },
		{ 0x0a20, "MSG_SENDCORPSE_EQ" },
		{ 0x9e40, "MSG_SENDPC_EQ" },
		{ 0x1520, "MSG_SENDPC_WEQ" },
		{ 0x4740, "MSG_SEND_CHARACTERS" },
		{ 0x0a41, "MSG_SEND_HDR" },
		{ 0x7c40, "MSG_SEND_INVENTORY" },
		{ 0x0441, "MSG_SEND_MONEY" },
		{ 0x0b41, "MSG" },
		{ 0x3c42, "MSG_SEND_PAGE_UPDATE" },
		{ 0x3640, "MSG_SEND_PC" },
		{ 0x8741, "MSG_SENSEDIRECTION" },
		{ 0x8841, "MSG_SENSETRAPS" },
		{ 0x2d20, "MSG_SERVERNAME" },
		{ 0x6f40, "MSG_SET_AVATAR" },
		{ 0xe841, "MSG_SET_DATARATE" },
		{ 0xfd41, "MSG_SET_FACTIONTABLE" },
		{ 0xdc41, "MSG_SET_MOTD" },
		{ 0x4b42, "MSG_SHIELD_PLAYER" },
		{ 0x1840, "MSG_SHOUT" },
		{ 0x0942, "MSG_SHOWINVISSHOUTS" },
		{ 0x3220, "MSG_SHUTDOWN_ALL" },
		{ 0xc740, "MSG_SILENCE_CLEAR" },
		{ 0x7940, "MSG_SILENCE_SET" },
		{ 0xd641, "MSG_SISALOG" },
		{ 0x8941, "MSG_SKILLIMPROVE" },
		{ 0xbe41, "MSG_SKILL_CHANGE" },
		{ 0x9641, "MSG_SKILL_IMPROVE" },
		{ 0x8d41, "MSG_SKY" },
		{ 0x5340, "MSG_SND_CORPSE_CON" },
		{ 0x5240, "MSG_SND_CORPSE_EQ" },
		{ 0x5540, "MSG_SND_CORPSE_KEY" },
		{ 0x5740, "MSG_SND_CORPSE_MAP" },
		{ 0x5640, "MSG_SND_CORPSE_NOTES" },
		{ 0x5440, "MSG_SND_CORPSE_SB" },
		{ 0x3040, "MSG_SND_ICON" },
		{ 0x6741, "MSG_SND_ICON_CRC" },
		{ 0x3140, "MSG_SND_IEQ" },
		{ 0x6841, "MSG_SND_IEQ_CRC" },
		{ 0x3340, "MSG_SND_IKEY" },
		{ 0x2f40, "MSG_SND_IMAP" },
		{ 0x3440, "MSG_SND_INOTES" },
		{ 0x6941, "MSG_SND_INOTES_CRC" },
		{ 0x3240, "MSG_SND_ISBOOK" },
		{ 0x0420, "MSG_SND_ITEM" },
		{ 0x0c40, "MSG_SND_MERCHANT_EQ" },
		{ 0x0940, "MSG_SND_MONEY" },
		{ 0x0220, "MSG_SND_NPC" },
		{ 0x0240, "MSG_SND_PCON" },
		{ 0x0340, "MSG_SND_PEQ" },
		{ 0x0540, "MSG_SND_PKEY" },
		{ 0x0140, "MSG_SND_PMAP" },
		{ 0x0640, "MSG_SND_PNOTES" },
		{ 0x0440, "MSG_SND_PSBOOK" },
		{ 0x1742, "MSG_SND_TRADER_EQ" },
		{ 0xfa40, "MSG_SND_WCON" },
		{ 0xfb40, "MSG_SND_WEQ" },
		{ 0xfc40, "MSG_SND_WNOTES" },
		{ 0x0a40, "MSG_SND_WOBJECTS" },
		{ 0x8541, "MSG_SNEAK" },
		{ 0x0241, "MSG_SNOOP_CLEAR" },
		{ 0x7a40, "MSG_SNOOP_SET" },
		{ 0x3341, "MSG_SNOOP_TEXT" },
		{ 0x1540, "MSG_SOCIAL" },
		{ 0xa841, "MSG_SOUL_MARK" },
		{ 0x6642, "MSG_SPELLACTIVATEPARTICLES" },
		{ 0x6742, "MSG_SPELLACTIVATEPARTICLESARRAY" },
		{ 0x2141, "MSG_SPELLCAST_OK" },
		{ 0x3541, "MSG_SPELLFILE_CHECK" },
		{ 0x4142, "MSG_SPELL_FIZZLE" },
		{ 0xd341, "MSG_SPELL_TEXT" },
		{ 0x6542, "MSG_SPELLWORNOFF" },
		{ 0x3141, "MSG_SPLIT_MONEY" },
		{ 0x6641, "MSG_START_ICON" },
		{ 0x6a41, "MSG_START_ICON_CRC" },
		{ 0x6441, "MSG_START_IEQ" },
		{ 0x6b41, "MSG_START_IEQ_CRC" },
		{ 0x6541, "MSG_START_INOTES" },
		{ 0x6c41, "MSG_START_INOTES_CRC" },
		{ 0xec40, "MSG_START_ROUTE" },
		{ 0xdd40, "MSG_START_TRADE" },
		{ 0xf540, "MSG_STAT_CHANGE" },
		{ 0xad40, "MSG_STEAL" },
		{ 0x7f41, "MSG_STOP_CASTING" },
		{ 0x5b41, "MSG_STUN_PLAYER" },
		{ 0xb340, "MSG_SUBMIT_BUG" },
		{ 0x3c41, "MSG_SUBMIT_FEEDBACK" },
		{ 0x0d10, "MSG_SUBMIT_REQUEST" },
		{ 0x5840, "MSG_SUCCESSFUL_HIT" },
		{ 0x4b41, "MSG_SUCCESSFUL_SKILL_USE" },
		{ 0xc540, "MSG_SUMMON_PLAYER" },
		{ 0xc441, "MSG_SURNAME" },
		{ 0xce41, "MSG_SWAP_SPELL" },
		{ 0x2241, "MSG_SWITCHSPELLNUM" },
		{ 0x8e40, "MSG_SWITCH_STATE" },
		{ 0x4842, "MSG_TAGSHOUT" },
		{ 0xb440, "MSG_TELEPORT_INDEX" },
		{ 0x4d41, "MSG_TELEPORT_PC" },
		{ 0xde41, "MSG_TELLTOGGLE" },
		{ 0x1440, "MSG_TEXT" },
		{ 0x6241, "MSG_TGTID" },
		{ 0x3320, "MSG_THREADSTATUS" },
		{ 0xf240, "MSG_TIME_STAMP" },
		{ 0x7040, "MSG_TOGGLE_CHANNEL" },
		{ 0xca40, "MSG_TOGGLE_PKILL" },
		{ 0x8d40, "MSG_TOGGLE_SWITCH" },
		{ 0x4342, "MSG_TOKEN_SOCIAL" },
		{ 0x3542, "MSG_TOKEN_TEXT" },
		{ 0x3642, "MSG_TOKEN_TEXT_PARAM" },
		{ 0x3e70, "MSG_TOUCHLIST" },
		{ 0x2640, "MSG_TRACELOGIN" },
		{ 0x8441, "MSG_TRACK" },
		{ 0x1040, "MSG_TRADEBUFFER_RESET" },
		{ 0x3e41, "MSG_TRADEFINAL_IEQ" },
		{ 0x3d41, "MSG_TRADEFINAL_MONEY" },
		{ 0x1842, "MSG_TRADER" },
		{ 0x1242, "MSG_TRADER_MANAGE" },
		{ 0x9640, "MSG_TRADE_ICON" },
		{ 0xdf40, "MSG_TRADE_IEQ" },
		{ 0xe240, "MSG_TRADE_IKEY" },
		{ 0xde40, "MSG_TRADE_IMAP" },
		{ 0xe340, "MSG_TRADE_INOTES" },
		{ 0xe040, "MSG_TRADE_ISBOOK" },
		{ 0xe440, "MSG_TRADE_MONEY" },
		{ 0x4041, "MSG_TRAIN" },
		{ 0x0280, "MSG_TRANSFER_PC" },
		{ 0x0780, "MSG_TRANSFER_PC_FORCED" },
		{ 0x0642, "MSG_TRANSLOCATE" },
		{ 0xf441, "MSG_TRAP_LOCATION" },
		{ 0x2a42, "MSG_TUNE_NPC" },
		{ 0xc140, "MSG_UNCONTROL_PLAYER" },
		{ 0x6b40, "MSG_UNFREEZE_PLAYER" },
		{ 0x4440, "MSG_UNGROUP" },
		{ 0x4140, "MSG_UNINVITE" },
		{ 0x4f40, "MSG_UNLOCK_CORPSE" },
		{ 0x1341, "MSG_UNLOCK_HOUSECHEST" },
		{ 0x3a40, "MSG_UNLOCK_MERCHANT" },
		{ 0x1542, "MSG_UPDATE_ALT_ABILS" },
		{ 0x9f40, "MSG_UPDATE_BUFFER" },
		{ 0xff41, "MSG_UPDATE_FILTERS" },
		{ 0x7d41, "MSG_UPDATE_LASTNAME" },
		{ 0x2542, "MSG_UPDATE_LUCLIN_FACE" },
		{ 0x1942, "MSG_UPDATE_MANA" },
		{ 0x2142, "MSG_UPDATE_MYCORPSE" },
		{ 0x4442, "MSG_UPDATE_PET_INFO" },
		{ 0xf340, "MSG_UPDATE_STATS" },
		{ 0x4d40, "MSG_UPD_CORPSE" },
		{ 0xfe40, "MSG_USER_CREATED" },
		{ 0x6242, "MSG_USER_DEL_PETITION_REQUEST" },
		{ 0x0e40, "MSG_USER_PETITION" },
		{ 0x6142, "MSG_USER_VIEW_PETITION_REQUEST" },
		{ 0x6342, "MSG_USER_VIEW_PETITION_RESPONSE" },
		{ 0x0f10, "MSG_VALID_PASSWD" },
		{ 0x0f20, "MSG_VEHICLE_FOUND" },
		{ 0x4a41, "MSG_VEHICLE_RESET" },
		{ 0xa240, "MSG_VEHICLE_XFR" },
		{ 0xcd41, "MSG_VIEW_ICON" },
		{ 0xca41, "MSG_VIEW_IEQ" },
		{ 0xcb41, "MSG_VIEW_INOTES" },
		{ 0xc741, "MSG_VIEW_PC" },
		{ 0x2120, "MSG_WCLEAR_FACTIONTABLE" },
		{ 0x8a41, "MSG_WEATHER" },
		{ 0x3641, "MSG_WEATHER_EVENT" },
		{ 0x2220, "MSG_WGET_SAFECOORDS" },
		{ 0x0b20, "MSG_WHO_RESPONSE" },
		{ 0xad41, "MSG_WIPE_INVENTORY" },
		{ 0x2a20, "MSG_WLDCLIENT_TEXT" },
		{ 0x2620, "MSG_WLDGROUP" },
		{ 0x3470, "MSG_WORLDPLAYERSTATS" },
		{ 0xed41, "MSG_WORLDSERVER_REJECT" },
		{ 0x3a20, "MSG_WORLD_REMOVE_GUILD" },
		{ 0x3b20, "MSG_WORLD_REMOVE_GUILD_RESPONSE" },
		{ 0x3f20, "MSG_WORLD_XFER" },
		{ 0x1d20, "MSG_WSERVER_SHUTDOWN" },
		{ 0xda41, "MSG_YELL" },
		{ 0x5b40, "MSG_ZHDR_REC" },
		{ 0x5d40, "MSG_ZHDR_REQ" },
		{ 0x8b41, "MSG_ZONECMD" },
		{ 0xdb41, "MSG_ZONECMDW" },
		{ 0xd541, "MSG_ZONECMDX" },
		{ 0x1620, "MSG_ZONECONTROL_PC" },
		{ 0x3820, "MSG_ZONEDONE_PC" },
		{ 0x2870, "MSG_ZONESTATUSREPLY" },
		{ 0x3970, "MSG_ZONESTATUSREPLY2" },
		{ 0x2770, "MSG_ZONESTATUSREQUEST" },
		{ 0x3870, "MSG_ZONESTATUSREQUEST2" },
		{ 0x0480, "MSG_ZONE_ADDRESS" },
		{ 0xa840, "MSG_ZONE_ALL" },
		{ 0xb040, "MSG_ZONE_HANDOVER_PC" },
		{ 0xe241, "MSG_ZONE_SKY" },
		{ 0x0580, "MSG_ZONE_UNAVAILABLE" },
		{ 0x1820, "MSG_ZREQ_LOOTERS" },
		{ 0x2720, "MSG_ZSERVER_ADDGROUPMEM" },
		{ 0x1a20, "MSG_ZSERVER_APP_ALIVE" },
		{ 0x1e20, "MSG_ZSERVER_CRASH" },
		{ 0x3c20, "MSG_ZSERVER_CREATEGRP" },
		{ 0x2820, "MSG_ZSERVER_DELGROUPMEM" },
		{ 0x2920, "MSG_ZSERVER_DISBAND" },
		{ 0x1120, "MSG_ZSERVER_READY" },
		{ 0xa340, "MSG_ZSERVER_STATUS" },
		};
	}
}

#endif

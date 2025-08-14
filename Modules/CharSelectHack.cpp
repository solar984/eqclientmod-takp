#include "eqclientmod.h"

// This hack makes the character selection screen rotation smooth.
// I play with uncapped framerate and this looks a lot nicer than the original jerky way.

#ifdef CHARSEL_HACK
#include "common.h"
#include "settings.h"
#include "util.h"

void ProcessCharacterSelectRotation()
{
	char *rotation = (char *)((*(int *)0x0063D5D8) + 368);
	uint32 *cur_gametime = (uint32 *)(*(intptr_t *)0x007F9510 + 200);
	static uint32 last_gametime = *cur_gametime;
	int *bThirdPerson = (int *)0x0063BE68;
	DWORD *dword_79968C = (DWORD *)0x0079968C;
	int LocalPlayer = 0x007F94CC;
	auto CDisplay__FixHeading = (double(__stdcall *)(float))0x004A2EED;


	if (*rotation)
	{
		float add = (*cur_gametime - last_gametime) * 0.06f; // add 0.06 per millisec elapsed
		float heading = *(float *)&dword_79968C[7 * *bThirdPerson];
		*(float *)&dword_79968C[7 * *bThirdPerson] = (float)CDisplay__FixHeading(heading + add);
		//Log("Rotation: %d, cur_gametime: %u, bThirdPerson: %d: heading %f, add %f, result %f", *rotation, *cur_gametime, *bThirdPerson, heading, add, *(float *)&dword_79968C[7 * *bThirdPerson]);
	}
	else
	{
		dword_79968C[7 * *bThirdPerson] = *(DWORD *)(*(intptr_t *)LocalPlayer + 84);
		//Log("Rotation: %d, cur_gametime: %u, bThirdPerson: %d", *rotation, *cur_gametime, *bThirdPerson);
	}

	last_gametime = *cur_gametime;
}

void ProcessCharacterCreationRotation()
{
	auto sub_49FA4E = (int *(__stdcall *)(int))0x0049FA4E;
	auto sub_52049A = (char(__thiscall *)(int))0x0052049A;
	char *rotation1 = (char *)((*(int *)0x0063D5DC) + 308);
	char *rotation2 = (char *)((*(int *)0x0063D63C) + 964);
	uint32 *cur_gametime = (uint32 *)(*(intptr_t *)0x007F9510 + 200);
	static uint32 last_gametime = *cur_gametime;
	int *bThirdPerson = (int *)0x0063BE68;
	DWORD *dword_79968C = (DWORD *)0x0079968C;
	int LocalPlayer = 0x007F94CC;
	auto CDisplay__FixHeading = (double(__stdcall *)(float))0x004A2EED;

	if (!*rotation1) // full character mode
	{
		if (*rotation2)
		{
			float add = (*cur_gametime - last_gametime) * 0.06f; // add 0.06 per millisec elapsed
			float heading = *(float *)&dword_79968C[7 * *bThirdPerson];
			*(float *)&dword_79968C[7 * *bThirdPerson] = (float)CDisplay__FixHeading(heading + add);
		}
		else
		{
			dword_79968C[7 * *bThirdPerson] = *(DWORD *)(*(intptr_t *)LocalPlayer + 84);
		}
	}
	else
	{
		sub_49FA4E(2); // zoom in on face
		if (*(DWORD *)(*(int *)0x0063D5DC + 412) != 2)
		{
			dword_79968C[7 * *bThirdPerson] = *(DWORD *)(*(intptr_t *)LocalPlayer + 84);
		}
		else
		{
			if ((unsigned __int8)sub_52049A(*(int *)LocalPlayer) == 1) // face edit mode - rotating when hair changed
			{
				float add = (*cur_gametime - last_gametime) * 0.045f; // add 0.045 per millisec elapsed, should be 0.03 to match original rate but that looked too jerky
				float heading = *(float *)&dword_79968C[7 * *bThirdPerson];
				*(float *)&dword_79968C[7 * *bThirdPerson] = (float)CDisplay__FixHeading(heading + add);
			}
		}
	}

	last_gametime = *cur_gametime;
}

void LoadCharSelectHack()
{
	bool enable = true;

#ifdef INI_FILE
	char buf[2048];
	const char *desc = "This hack makes the character selection screen rotation smooth.  If you are using some kind of FPS limiter I recommend not doing that, because this looks way better.";
	WritePrivateProfileStringA("CharSelect", "Description", desc, INI_FILE);
	GetINIString("CharSelect", "Enabled", "TRUE", buf, sizeof(buf), true);
	enable = ParseINIBool(buf);
#endif

	Log("LoadCharSelectHack(): hack is %s", enable ? "ENABLED" : "DISABLED");

	if (enable)
	{

		// this is for smoothing out rotation on the normal character selection rotation
		{
			// 0x0053C2E2 - 0x0053C341 len 95
			/*
			  cur_gametime = *((_DWORD *)DisplayObject + 50);
			  if ( cur_gametime > next_gametime )
			  {
				v17 = *(_BYTE *)(dword_63D5D8 + 368) == 0;
				next_gametime = cur_gametime + 50;
				if ( v17 )
				{
				  dword_79968C[7 * bThirdPerson] = LODWORD(LocalPlayer->heading);
				}
				else
				{
				  v31 = *(float *)&dword_79968C[7 * bThirdPerson] + 3.0;
				  *(float *)&dword_79968C[7 * bThirdPerson] = CDisplay::FixHeading(v31);
				}
			  }
			*/

			unsigned char expectedCode[] = { 0x8B, 0x80, 0xC8, 0x00, 0x00, 0x00, 0x3B, 0x45, 0xF0, 0x76, 0x54, 0x83, 0xC0, 0x32, 0x80, 0xB9 };
			if (memcmp((void *)0x0053C2E2, expectedCode, sizeof(expectedCode)))
			{
				Log("LoadCharSelectHack(): program doesn't match expected value, not rewriting.");
				return;
			}
			unsigned char code[] =
			{
				0xE8, 0x00, 0x00, 0x00, 0x00,	// call ProcessCharacterSelectRotation
				0xB8, 0x41, 0xC3, 0x53, 0x00,	// mov eax, 0x0053C341
				0xFF, 0xE0						// jmp eax
			};
			*(intptr_t *)(code + 1) = (intptr_t)ProcessCharacterSelectRotation - (intptr_t)0x0053C2E2 - 5;
			Patch((void *)0x0053C2E2, code, sizeof(code));

			// Sleep(10) -> Sleep(0)
			// text:0053C46C 134 6A 0A                                         push    0Ah             ; dwMilliseconds
			char dwMilliseconds = 0;
			Patch((void *)(0x0053C46C + 1), &dwMilliseconds, 1);
		}

		// this is for character creation rotation
		{
			// 0x0053AF21 - 0x0053AFD9 len 184
			/*
				v4 = *((_DWORD *)DisplayObject + 50);
				if ( v4 <= v13 )
				  goto LABEL_16;
				v13 = v4 + 50;

				if ( !*(_BYTE *)(dword_63D5DC + 308) )
				{
				  v5 = bThirdPerson;
				  if ( *(_BYTE *)(dword_63D63C + 964) )
				  {
					v6 = *(float *)&dword_79968C[7 * bThirdPerson] + 3.0;
					v10 = v6;
			LABEL_10:
					CDisplay::FixHeading(v10);
					*(float *)&dword_79968C[7 * bThirdPerson] = v6;
					goto LABEL_16;
				  }
				  goto LABEL_15;
				}

				if ( *(_DWORD *)(dword_63D5DC + 412) != 2 )
				{
				  v5 = bThirdPerson;
			LABEL_15:
				  dword_79968C[7 * v5] = LODWORD(LocalPlayer->heading);
				  goto LABEL_16;
				}

				sub_49FA4E(2);

				if ( (unsigned __int8)sub_52049A(LocalPlayer) == 1 )
				{
				  v6 = *(float *)&dword_79968C[7 * bThirdPerson] + 1.5;
				  v10 = v6;
				  goto LABEL_10;
				}
			*/

			unsigned char code[] =
			{
				0xE8, 0x00, 0x00, 0x00, 0x00,	// call ProcessCharacterSelectRotation
				0xB8, 0xD9, 0xAF, 0x53, 0x00,	// mov eax, 0x0053AFD9
				0xFF, 0xE0						// jmp eax
			};
			*(intptr_t *)(code + 1) = (intptr_t)ProcessCharacterCreationRotation - (intptr_t)0x0053AF21 - 5;
			Patch((void *)0x0053AF21, code, sizeof(code));

			// Sleep(10) -> Sleep(0)
			// .text:0053B064 0A0 6A 0A                                         push    0Ah             ; dwMilliseconds
			char dwMilliseconds = 0;
			Patch((void *)(0x0053B064 + 1), &dwMilliseconds, 1);
		}
	}
}

#endif
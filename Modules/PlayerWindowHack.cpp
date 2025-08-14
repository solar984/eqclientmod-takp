#include "eqclientmod.h"

// This makes the auto attack status more visible in the border of the player window (the one with your name and health/mana)

#ifdef PLAYERWINDOW_ATTACK_INDICATOR
#include "settings.h"
#include "util.h"

#include <algorithm>

#define EQ_IS_AUTO_ATTACK_ENABLED (*(char *)0x007F6FFE)

// CXWnd::DrawColoredRect
typedef int(__cdecl *_CXWnd__DrawColoredRect)(int x1, int y1, int x2, int y2, int color, int clip_x1, int clip_y1, int clip_x2, int clip_y2);
_CXWnd__DrawColoredRect CXWnd__DrawColoredRect = (_CXWnd__DrawColoredRect)0x00574380;

// CXWnd::GetClientClipRect
typedef int(__thiscall *_CXWnd__GetClientClipRect)(void *this_ptr, int *rect);
_CXWnd__GetClientClipRect CXWnd__GetClientClipRect = (_CXWnd__GetClientClipRect)0x00574C10;

int DrawLasso(int x1, int y1, int x2, int y2, int color, int clip_x1, int clip_y1, int clip_x2, int clip_y2, int thickness)
{
	if (x1 > x2)
	{
		int left = x2;
		x2 = x1;
		x1 = left;
	}

	if (y1 > y2)
	{
		int top = y1;
		y1 = y2;
		y2 = top;
	}

	void *CXWnd__sm_pMgr = **(void ***)0x809C34;
	int screen_width = *(int *)((int)CXWnd__sm_pMgr + 188);
	int screen_height = *(int *)((int)CXWnd__sm_pMgr + 192);
	int clip_right = std::min(clip_x2, screen_width);
	int clip_bottom = std::min(clip_y2, screen_height);

	int vthickness = std::min(thickness, (y2 - y1) / 2);
	int hthickness = std::min(thickness, (x2 - x1) / 2);

	if (CXWnd__DrawColoredRect(x1, y1, x1 + hthickness, y2, color, clip_x1, clip_y1, clip_right, clip_bottom) < 0	// left
		|| CXWnd__DrawColoredRect(x1 + hthickness, y1, x2 - hthickness, y1 + vthickness, color, clip_x1, clip_y1, clip_right, clip_bottom) < 0	// top
		|| CXWnd__DrawColoredRect(x1 + hthickness, y2 - vthickness, x2 - hthickness, y2, color, clip_x1, clip_y1, clip_right, clip_bottom) < 0	// bottom
		|| CXWnd__DrawColoredRect(x2 - hthickness, y1, x2, y2, color, clip_x1, clip_y1, clip_right, clip_bottom) < 0)	// right
	{
		return -1;
	}

	return 0;
}

//typedef int (__thiscall  *_CPlayerWnd__Draw_Trampoline)(void *this_ptr);
//_CPlayerWnd__Draw_Trampoline CPlayerWnd__Draw_Trampoline;

static uint32_t rgb_rainbow(double ratio)
{
	//we want to normalize ratio so that it fits in to 6 regions
	//where each region is 256 units long
	int normalized = (int)(ratio * 256 * 6);

	//find the region for this position
	int region = normalized / 256;

	//find the distance to the start of the closest region
	int ofs = normalized % 256;

	uint8_t r = 0, g = 0, b = 0;
	switch (region)
	{
	case 0: r = 255; g = 0;   b = 0;   g += ofs; break;
	case 1: r = 255; g = 255; b = 0;   r -= ofs; break;
	case 2: r = 0;   g = 255; b = 0;   b += ofs; break;
	case 3: r = 0;   g = 255; b = 255; g -= ofs; break;
	case 4: r = 0;   g = 0;   b = 255; r += ofs; break;
	case 5: r = 255; g = 0;   b = 255; b -= ofs; break;
	}
	return b + (g << 8) + (r << 16) + 0xff000000;
}

static uint32_t rgb_redwhite(double ratio)
{
	int normalized = (int)(ratio * 256 * 2);
	int region = normalized / 256;
	int ofs = normalized % 256;

	uint8_t r = 0, g = 0, b = 0;
	switch (region)
	{
	case 0: r = 255; g = 0;   b = 0;   b += ofs * 175 / 256; g += ofs * 175 / 256; break;
	case 1: r = 255; g = 175; b = 175; b -= ofs * 175 / 256; g -= ofs * 175 / 256; break;
	}
	return b + (g << 8) + (r << 16) + 0xFF000000;
}

int lasso_thickness = 3;
int color_function = 1;
float color_speed = 8.0f;
int __fastcall CPlayerWnd__Draw_Detour(void *this_ptr)
{
	static bool reset_state = 1;
	static int rainbow = 0;

	if (EQ_IS_AUTO_ATTACK_ENABLED)
	{
		reset_state = 1;

		uint32 *cur_gametime = (uint32 *)(*(intptr_t *)0x007F9510 + 200);
		static uint32 last_gametime = *cur_gametime;
		float add = (*cur_gametime - last_gametime) * color_speed;
		last_gametime = *cur_gametime;
		int cycle_scale = 10000;
		DWORD current_color;
		switch (color_function)
		{
		case 2:
			current_color = rgb_rainbow(rainbow / (float)cycle_scale);
			break;
		case 1:
		default:
			current_color = rgb_redwhite(rainbow / (float)cycle_scale);
			break;
		}
		rainbow = (int)(rainbow + add);
		if (rainbow >= cycle_scale)
			rainbow = 0;

		int lasso_rect[4];
		int clip_rect[4];

		// virtual function in vtable
		// CXWnd::GetInnerRect(void)
		(*(void(__thiscall **)(void *, int *))(*(DWORD *)this_ptr + 216))(this_ptr, lasso_rect);
		CXWnd__GetClientClipRect(this_ptr, clip_rect);

		DrawLasso(lasso_rect[0], lasso_rect[1], lasso_rect[2], lasso_rect[3], current_color, clip_rect[0], clip_rect[1], clip_rect[2], clip_rect[3], lasso_thickness);
	}
	else if (reset_state)
	{
		// reset to initial state so it starts the animation the same way each time attack is activated
		rainbow = 0;
		reset_state = 0;
	}

	return 0;
}
#if false
int __fastcall CPlayerWnd__Draw_Detour(void *this_ptr)
{
	static unsigned char alpha = 0xFF;
	static char fade = 1;
	static bool reset_state = 1;

	DWORD lasso_color = 0x00FF0000; // ARGB
	char lasso_min_alpha = 40;
	char lasso_fade_speed = 2;
	int lasso_thickness = 3;

	if (EQ_IS_AUTO_ATTACK_ENABLED)
	{
		reset_state = 1;
		DWORD current_color = lasso_color & 0x00FFFFFF | (alpha << 24);

		int lasso_rect[4];
		int clip_rect[4];

		// virtual function in vtable
		// CXWnd::GetInnerRect(void)
		(*(void(__thiscall **)(void *, int *))(*(DWORD *)this_ptr + 216))(this_ptr, lasso_rect);
		CXWnd__GetClientClipRect(this_ptr, clip_rect);
		DrawLasso(lasso_rect[0], lasso_rect[1], lasso_rect[2], lasso_rect[3], current_color, clip_rect[0], clip_rect[1], clip_rect[2], clip_rect[3], lasso_thickness);

		// advance alpha value for next call
		char add = (fade ? -1 : 1) * lasso_fade_speed;
		if (add > 0 && (unsigned char)(alpha + add) < alpha)
		{
			fade = 1;
			add *= -1;
		}
		if (add < 0 && (unsigned char)(alpha + add) > alpha || alpha + add < lasso_min_alpha)
		{
			fade = 0;
			add *= -1;
		}
		alpha += add;
	}
	else if (reset_state)
	{
		// reset to initial state so it starts the animation the same way each time attack is activated
		alpha = 0xFF;
		fade = 1;
		reset_state = 0;
	}

	return 0;
}
#endif

void LoadPlayerWindowHack()
{
	bool enable = true;

#ifdef INI_FILE
	char buf[2048];
	const char *desc = "This makes the auto attack status more visible in the border of the player window (the one with your name and health/mana).  The rectangle around the inside of the window is called a lasso here, you can use LassoThickness to adjust how thick the rectangle is.  Set ColorFunction to 1 for a red/white pulsing effect or 2 for a rainbow effect.  Set ColorSpeed to control how fast the colors cycle.";
	WritePrivateProfileStringA("PlayerWindow", "Description", desc, INI_FILE);
	GetINIString("PlayerWindow", "Enabled", "TRUE", buf, sizeof(buf), true);
	enable = ParseINIBool(buf);
	GetINIString("PlayerWindow", "LassoThickness", "3", buf, sizeof(buf), true);
	lasso_thickness = atoi(buf);
	GetINIString("PlayerWindow", "ColorFunction", "1", buf, sizeof(buf), true);
	color_function = atoi(buf);
	GetINIString("PlayerWindow", "ColorSpeed", "8.0", buf, sizeof(buf), true);
	color_speed = (float)atof(buf);
#endif

	Log("LoadPlayerWindowHack(): hack is %s, lasso_thickness is %d, color_function is %d, color_speed is %0.2f", enable ? "ENABLED" : "DISABLED", lasso_thickness, color_function, color_speed);

	Detour((void *)0x0042EDE3, (void *)CPlayerWnd__Draw_Detour);
}
#endif

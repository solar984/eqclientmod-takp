#pragma once

#define WIN32_LEAN_AND_MEAN
#define WINDOWS_IGNORE_PACKING_MISMATCH
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>

#include <stdint.h>
typedef uint8_t byte;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;


#define INI_FILE ".\\eqclientmod.ini"

#define BUILD_VERSION "SOLAR_TAKP_2025111500"
#define COMMAND_HANDLER //
#define PLAYERWINDOW_ATTACK_INDICATOR //
#define FOV_HACK //
#define AUTOFIRE_HACK //
#define CHARSEL_HACK //
#define AUTOFOLLOW_HACK //
#define HIDECORPSE_HACK //
#define FARCLIP_HACK //
#define TRIGFUNCTIONS_HACK //
#define GAMMA_HACK //
#define FONT_HACK //
#define NAMEPLATE_HACK

#define BUFFWINDOW_HACK

//#define BUFFS
#define DEBUG_HACK

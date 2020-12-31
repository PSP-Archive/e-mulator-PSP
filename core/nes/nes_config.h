/*
** nester - NES emulator
** Copyright (C) 2000  Darren Ranalli
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.  To obtain a
** copy of the GNU Library General Public License, write to the Free
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
*/

#ifndef NES_CONFIG_H_
#define NES_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	FILTER_NONE, FILTER_LOWPASS, FILTER_LOWPASS_WEIGHTED, FILTER_OLOWPASS
}filter_type_t;

// 注意：下記構造体を変更したらmain.hの NESTERJ_CFG_FORMATVERSION の値を変更して下さい。
// 変更しないとそれ以前のNESTERJ.CFGの入っているPSPでハングアップする恐れあり。

typedef struct //_NES_CONFIG
{
	// preferences
	struct {
		unsigned char	TV_Mode;  // 0 = Auto, 1 = NTSC, 2 = PAL
		u8 UseFDSDiskASkip;
		u8 UseRomDataBase;
	}preferences;
	// graphics
	struct {
		u8 black_and_white;
		u8 show_all_scanlines;
		u8 DisableSpriteClipping;
		u8 DisableBackGClipping;
		u8 DefaultPaletteNo;
		u8 show_more_than_8_sprites;
	}graphics;
	// sound
	struct {
		u8 enabled;
		u8 rectangle1_enabled;
		u8 rectangle2_enabled;
		u8 triangle_enabled;
		u8 noise_enabled;
		u8 dpcm_enabled;
		u8 ext_enabled;

		filter_type_t filter_type;
		u32 lowpass_filter_f;

		u8 ideal_triangle_enabled;
		u8 smooth_envelope_enabled; // reserved
		u8 smooth_sweep_enabled; // reserved
	}sound;
}NES_CONFIG;

extern NES_CONFIG g_NESConfig; // NES の設定

void NES_Config_SetDefaults_All(void);

void NES_Config_SetPreferencesDefaults(void);

void NES_Config_SetGraphicsDefaults(void);

void NES_Config_SetSoundDefaults(void);

#ifdef __cplusplus
}
#endif

#endif

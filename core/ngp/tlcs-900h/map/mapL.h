//---------------------------------------------------------------------------
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version. See also the license.txt file for
//	additional informations.
//---------------------------------------------------------------------------

/*
//---------------------------------------------------------------------------
//=========================================================================

	registers_mapL.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

//---------------------------------------------------------------------------
*/

	//Bank 0
	{
		(u32*)&(gprBank[0][0]),
		(u32*)&(gprBank[0][1]),
		(u32*)&(gprBank[0][2]),
		(u32*)&(gprBank[0][3]),
		(u32*)&(gpr[0]),
		(u32*)&(gpr[1]),
		(u32*)&(gpr[2]),
		(u32*)&(gpr[3]),
	},

	//Bank 1
	{
		(u32*)&(gprBank[1][0]),
		(u32*)&(gprBank[1][1]),
		(u32*)&(gprBank[1][2]),
		(u32*)&(gprBank[1][3]),
		(u32*)&(gpr[0]),
		(u32*)&(gpr[1]),
		(u32*)&(gpr[2]),
		(u32*)&(gpr[3]),
	},
	
	//Bank 2
	{
		(u32*)&(gprBank[2][0]),
		(u32*)&(gprBank[2][1]),
		(u32*)&(gprBank[2][2]),
		(u32*)&(gprBank[2][3]),
		(u32*)&(gpr[0]),
		(u32*)&(gpr[1]),
		(u32*)&(gpr[2]),
		(u32*)&(gpr[3]),
	},

	//Bank 3
	{
		(u32*)&(gprBank[3][0]),
		(u32*)&(gprBank[3][1]),
		(u32*)&(gprBank[3][2]),
		(u32*)&(gprBank[3][3]),
		(u32*)&(gpr[0]),
		(u32*)&(gpr[1]),
		(u32*)&(gpr[2]),
		(u32*)&(gpr[3]),
	},

//=============================================================================



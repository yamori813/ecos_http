/****************************************************************************** 
* 
* Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved. 
* 
* This program is free software; you can redistribute it and/or modify it 
* under the terms of version 2 of the GNU General Public License as 
* published by the Free Software Foundation. 
* 
* This program is distributed in the hope that it will be useful, but WITHOUT 
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for 
* more details. 
* 
* You should have received a copy of the GNU General Public License along with 
* this program; if not, write to the Free Software Foundation, Inc., 
* 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA 
* 
* 
******************************************************************************/

#if (RTL8821B_SUPPORT == 1)
#ifndef __INC_TC_BB_HW_IMG_8821B_H
#define __INC_TC_BB_HW_IMG_8821B_H


/******************************************************************************
*                           AGC_TAB.TXT
******************************************************************************/

void
ODM_ReadAndConfig_TC_8821B_AGC_TAB( // TC: Test Chip, MP: MP Chip
	IN   PDM_ODM_T  pDM_Odm
);
u4Byte
ODM_GetVersion_TC_8821B_AGC_TAB(
);

/******************************************************************************
*                           PHY_REG.TXT
******************************************************************************/

void
ODM_ReadAndConfig_TC_8821B_PHY_REG( // TC: Test Chip, MP: MP Chip
	IN   PDM_ODM_T  pDM_Odm
);
u4Byte
ODM_GetVersion_TC_8821B_PHY_REG(
);

/******************************************************************************
*                           PHY_REG_MP.TXT
******************************************************************************/

void
ODM_ReadAndConfig_TC_8821B_PHY_REG_MP( // TC: Test Chip, MP: MP Chip
	IN   PDM_ODM_T  pDM_Odm
);
u4Byte
ODM_GetVersion_TC_8821B_PHY_REG_MP(
);

/******************************************************************************
*                           PHY_REG_PG.TXT
******************************************************************************/

void
ODM_ReadAndConfig_TC_8821B_PHY_REG_PG( // TC: Test Chip, MP: MP Chip
	IN   PDM_ODM_T  pDM_Odm
);
u4Byte
ODM_GetVersion_TC_8821B_PHY_REG_PG(
);

#endif
#endif // end of HWIMG_SUPPORT


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

/*============================================================
// include files
============================================================*/

#include "mp_precomp.h"
#include "../phydm_precomp.h"

#if (RTL8197F_SUPPORT == 1)

s1Byte 
odm_CCKRSSI_8197F(
	IN		PDM_ODM_T		pDM_Odm,
	IN		u1Byte	LNA_idx, 
	IN		u1Byte	VGA_idx
	)
{
	s1Byte	rx_pwr_all;
	s1Byte	diff_para;

	if ((pDM_Odm->BoardType == (ODM_BOARD_EXT_TRSW|ODM_BOARD_EXT_LNA|ODM_BOARD_EXT_PA)) && (pDM_Odm->PackageType == 1))
		diff_para = -7;
	else
		diff_para = 7;
	
	switch (LNA_idx) {
	case 7:
		rx_pwr_all = -52 + diff_para  - 2*(VGA_idx);
		break;

	case 6:
		rx_pwr_all = -42 + diff_para - 2*(VGA_idx); 
		break;

	case 5:
		rx_pwr_all = -32 + diff_para - 2*(VGA_idx); 
		break;

	case 4:
		rx_pwr_all = -26 + diff_para - 2*(VGA_idx); 
		break;

	case 3:
		rx_pwr_all = -18 + diff_para - 2*(VGA_idx); 
		break;

	case 2:
		rx_pwr_all = -12 + diff_para - 2*(VGA_idx);
		break;

	case 1:
		rx_pwr_all = -2 + diff_para - 2*(VGA_idx);
		break;
		
	case 0:
		rx_pwr_all = 2 + diff_para - 2*(VGA_idx);
		break;

	default:
	break;			

	}

	return rx_pwr_all;

}

VOID
phydm_phypara_a_cut(
	IN		PDM_ODM_T		pDM_Odm
	)
{

	ODM_SetBBReg(pDM_Odm, 0x97c, 0xff000000, 0x20);/*97f A-cut workaround*/

	ODM_SetBBReg(pDM_Odm, 0xa9c, BIT17, 0);/*97f A-cut workaround*/
	ODM_SetBBReg(pDM_Odm, 0xa0c, bMaskByte2, 0x7e);/*97f A-cut workaround*/
	ODM_SetBBReg(pDM_Odm, 0xa0c, bMaskByte1, 0x0);/*97f A-cut workaround*/
	ODM_SetBBReg(pDM_Odm, 0xa84, bMaskByte2, 0x1f);/*97f A-cut workaround*/

	ODM_SetBBReg(pDM_Odm, 0x824, BIT14, 0x1);/*pathA r_rxhp_tx*/
	ODM_SetBBReg(pDM_Odm, 0x824, BIT17, 0x1);/*pathA r_rxhp_t2r*/
	ODM_SetBBReg(pDM_Odm, 0x82c, BIT14, 0x1);/*pathB r_rxhp_tx*/
	ODM_SetBBReg(pDM_Odm, 0x82c, BIT17, 0x1);/*pathB r_rxhp_t2r*/
	ODM_SetBBReg(pDM_Odm, 0xc5c, BIT2, 0x1);/*97f A-cut workaround*/
	ODM_SetBBReg(pDM_Odm, 0xc5c, BIT5, 0x1);/*97f A-cut workaround*/
	ODM_SetBBReg(pDM_Odm, 0xc5c, BIT8, 0x1);/*97f A-cut workaround*/
	ODM_SetBBReg(pDM_Odm, 0xc5c, BIT11, 0x1);/*97f A-cut workaround*/
	ODM_SetBBReg(pDM_Odm, 0xc5c, BIT14, 0x1);/*97f A-cut workaround*/
	ODM_SetBBReg(pDM_Odm, 0xce0, BIT2, 0x1);/*97f A-cut workaround*/
	ODM_SetBBReg(pDM_Odm, 0xc7c, BIT24, 0x1);/*97f A-cut workaround*/
	ODM_SetBBReg(pDM_Odm, 0xc7c, BIT27, 0x1);/*97f A-cut workaround*/

}

#endif	/* RTL8197F_SUPPORT == 1 */


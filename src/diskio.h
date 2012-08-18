/*
  Rugged Audio Shield Firmware for ATxmega

  Copyright (c) 2012 Rugged Circuits LLC.  All rights reserved.
  http://ruggedcircuits.com

  This file is part of the Rugged Circuits Rugged Audio Shield firmware distribution.

  This is free software; you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free Software
  Foundation; either version 3 of the License, or (at your option) any later
  version.

  This software is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  A copy of the GNU General Public License can be viewed at
  <http://www.gnu.org/licenses>

  Copyright (C) 2011, ChaN, all right reserved.

 * The FatFs module is a free software and there is NO WARRANTY.
 * No restriction on use. You can use, modify and redistribute it for
   personal, non-profit or commercial product UNDER YOUR RESPONSIBILITY.
 * Redistributions of source code must retain the above copyright notice.

*/
/*-----------------------------------------------------------------------
/  Low level disk interface module include file
/-----------------------------------------------------------------------*/

#ifndef _DISKIO_H_
#define _DISKIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "integer.h"
#include "functable.h"

/* Status of Disk Functions */
typedef BYTE	DSTATUS;

/* Results of Disk Functions */
typedef enum {
	RES_OK = 0,		/* 0: Successful */
	RES_ERROR,		/* 1: R/W Error */
	RES_WRPRT,		/* 2: Write Protected */
	RES_NOTRDY,		/* 3: Not Ready */
	RES_PARERR		/* 4: Invalid Parameter */
} DRESULT;

/*---------------------------------------*/
/* Prototypes for disk control functions */

typedef DSTATUS ( *disk_initialize_t) (BYTE);
#define disk_initialize(a) ((disk_initialize_t)(pgm_read_word(&_functable[_FT_disk_initialize])))((a))
typedef DSTATUS ( *disk_status_t) (BYTE);
#define disk_status(a) ((disk_status_t)(pgm_read_word(&_functable[_FT_disk_status])))((a))
typedef DRESULT ( *disk_read_t) (BYTE, BYTE*, DWORD, BYTE);
#define disk_read(a,b,c,d) ((disk_read_t)(pgm_read_word(&_functable[_FT_disk_read])))((a),(b),(c),(d))
typedef DRESULT ( *disk_write_t) (BYTE, const BYTE*, DWORD, BYTE);
#define disk_write(a,b,c,d) ((disk_write_t)(pgm_read_word(&_functable[_FT_disk_write])))((a),(b),(c),(d))
typedef DRESULT ( *disk_ioctl_t) (BYTE, BYTE, void*);
#define disk_ioctl(a,b,c) ((disk_ioctl_t)(pgm_read_word(&_functable[_FT_disk_ioctl])))((a),(b),(c))
typedef void ( *disk_timerproc_t) (void);
#define disk_timerproc() ((disk_timerproc_t)(pgm_read_word(&_functable[_FT_disk_timerproc])))()

/* Disk Status Bits (DSTATUS) */

#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */


/* Command code for disk_ioctrl fucntion */

/* Generic command (defined for FatFs) */
#define CTRL_SYNC			0	/* Flush disk cache (for write functions) */
#define GET_SECTOR_COUNT	1	/* Get media size (for only f_mkfs()) */
#define GET_SECTOR_SIZE		2	/* Get sector size (for multiple sector size (_MAX_SS >= 1024)) */
#define GET_BLOCK_SIZE		3	/* Get erase block size (for only f_mkfs()) */
#define CTRL_ERASE_SECTOR	4	/* Force erased a block of sectors (for only _USE_ERASE) */

/* Generic command */
#define CTRL_POWER			5	/* Get/Set power status */
#define CTRL_LOCK			6	/* Lock/Unlock media removal */
#define CTRL_EJECT			7	/* Eject media */

/* MMC/SDC specific ioctl command */
#define MMC_GET_TYPE		10	/* Get card type */
#define MMC_GET_CSD			11	/* Get CSD */
#define MMC_GET_CID			12	/* Get CID */
#define MMC_GET_OCR			13	/* Get OCR */
#define MMC_GET_SDSTAT		14	/* Get SD status */

/* ATA/CF specific ioctl command */
#define ATA_GET_REV			20	/* Get F/W revision */
#define ATA_GET_MODEL		21	/* Get model name */
#define ATA_GET_SN			22	/* Get serial number */

/* NAND specific ioctl command */
#define NAND_FORMAT			30	/* Create physical format */

#ifdef __cplusplus
}
#endif

#endif // _DISKIO_H_
// vim: expandtab ts=2 sw=2 ai cindent

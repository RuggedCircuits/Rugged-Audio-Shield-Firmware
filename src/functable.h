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
*/
#ifndef _FUNCTABLE_H_
#define _FUNCTABLE_H_

static const void * * const _functable = (const void **)0x8FC0;

// The order of functions here must match the order in which functions are stored in
// this table in the bootloader.
enum { 
  _FT_bootloader_version,
  _FT_f_chdir,
  _FT_f_chdrive,
  _FT_f_chmod,
  _FT_f_close,
  _FT_f_forward,
  _FT_f_getcwd,
  _FT_f_getfree,
  _FT_f_lseek,
  _FT_f_mkdir,
  _FT_f_mkfs,
  _FT_f_mount,
  _FT_f_open,
  _FT_f_opendir,
  _FT_f_read,
  _FT_f_readdir,
  _FT_f_rename,
  _FT_f_stat,
  _FT_f_sync,
  _FT_f_truncate,
  _FT_f_unlink,
  _FT_f_utime,
  _FT_f_write,
  _FT_disk_initialize,
  _FT_disk_ioctl,
  _FT_disk_read,
  _FT_disk_status,
  _FT_disk_write,
  _FT_disk_timerproc,
};

typedef uint16_t (*bootloader_version_t)(void);
#define bootloader_version() ((bootloader_version_t)(pgm_read_word(&_functable[_FT_bootloader_version])))()

#endif  // _FUNCTABLE_H_
// vim: expandtab ts=2 sw=2 ai cindent

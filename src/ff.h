/*---------------------------------------------------------------------------/
/  FatFs - FAT file system module include file  R0.09     (C)ChaN, 2011
/----------------------------------------------------------------------------/
/ FatFs module is a generic FAT file system module for small embedded systems.
/ This is a free software that opened for education, research and commercial
/ developments under license policy of following trems.
/
/  Copyright (C) 2011, ChaN, all right reserved.
/
/ * The FatFs module is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial product UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/ Modified by Rugged Circuits LLC 2012.03.08
/ Modifications are to make this header file an API to precompiled functions
/ residing in FLASH on a Rugged Audio Shield.
/----------------------------------------------------------------------------*/

#ifndef _FATFS
#define _FATFS  6502  /* Revision ID */

#ifdef __cplusplus
extern "C" {
#endif

#include "integer.h"  /* Basic integer types */
#include "ffconf.h"    /* FatFs configuration options */
#include "functable.h"

#if _FATFS != _FFCONF
#error Wrong configuration file (ffconf.h).
#endif

/* Type of path name strings on FatFs API */

#ifndef _INC_TCHAR
typedef char TCHAR;
#define _T(x) x
#define _TEXT(x) x
#endif

/* File system object structure (FATFS) */

typedef struct {
  BYTE  fs_type;    /* FAT sub-type (0:Not mounted) */
  BYTE  drv;      /* Physical drive number */
  BYTE  csize;      /* Sectors per cluster (1,2,4...128) */
  BYTE  n_fats;      /* Number of FAT copies (1,2) */
  BYTE  wflag;      /* win[] dirty flag (1:must be written back) */
  BYTE  fsi_flag;    /* fsinfo dirty flag (1:must be written back) */
  WORD  id;        /* File system mount ID */
  WORD  n_rootdir;    /* Number of root directory entries (FAT12/16) */
#if _MAX_SS != 512
#error Not supported
  WORD  ssize;      /* Bytes per sector (512, 1024, 2048 or 4096) */
#endif
#if _FS_REENTRANT
#error Not supported
  _SYNC_t  sobj;      /* Identifier of sync object */
#endif
#if !_FS_READONLY
  DWORD  last_clust;    /* Last allocated cluster */
  DWORD  free_clust;    /* Number of free clusters */
  DWORD  fsi_sector;    /* fsinfo sector (FAT32) */
#endif
#if _FS_RPATH
  DWORD  cdir;      /* Current directory start cluster (0:root) */
#endif
  DWORD  n_fatent;    /* Number of FAT entries (= number of clusters + 2) */
  DWORD  fsize;      /* Sectors per FAT */
  DWORD  fatbase;    /* FAT start sector */
  DWORD  dirbase;    /* Root directory start sector (FAT32:Cluster#) */
  DWORD  database;    /* Data start sector */
  DWORD  winsect;    /* Current sector appearing in the win[] */
  BYTE  win[_MAX_SS];  /* Disk access window for Directory, FAT (and Data on tiny cfg) */
} FATFS;

/* File object structure (FIL) */

typedef struct {
  FATFS*  fs;        /* Pointer to the owner file system object */
  WORD  id;        /* Owner file system mount ID */
  BYTE  flag;      /* File status flags */
  BYTE  pad1;
  DWORD  fptr;      /* File read/write pointer (0 on file open) */
  DWORD  fsize;      /* File size */
  DWORD  sclust;      /* File start cluster (0 when fsize==0) */
  DWORD  clust;      /* Current cluster */
  DWORD  dsect;      /* Current data sector */
#if !_FS_READONLY
  DWORD  dir_sect;    /* Sector containing the directory entry */
  BYTE*  dir_ptr;    /* Ponter to the directory entry in the window */
#endif
#if _USE_FASTSEEK
  DWORD*  cltbl;      /* Pointer to the cluster link map table (null on file open) */
#endif
#if _FS_SHARE
#error Not supported
  UINT  lockid;      /* File lock ID (index of file semaphore table) */
#endif
#if !_FS_TINY
#error Not supported
  BYTE  buf[_MAX_SS];  /* File data read/write buffer */
#endif
} FIL;

/* Directory object structure (DIR) */

typedef struct {
  FATFS*  fs;        /* Pointer to the owner file system object */
  WORD  id;        /* Owner file system mount ID */
  WORD  index;      /* Current read/write index number */
  DWORD  sclust;      /* Table start cluster (0:Root dir) */
  DWORD  clust;      /* Current cluster */
  DWORD  sect;      /* Current sector */
  BYTE*  dir;      /* Pointer to the current SFN entry in the win[] */
  BYTE*  fn;        /* Pointer to the SFN (in/out) {file[8],ext[3],status[1]} */
#if _USE_LFN
#error Not supported
  WCHAR*  lfn;      /* Pointer to the LFN working buffer */
  WORD  lfn_idx;    /* Last matched LFN index number (0xFFFF:No LFN) */
#endif
} DIR;

/* File status structure (FILINFO) */

typedef struct {
  DWORD  fsize;      /* File size */
  WORD  fdate;      /* Last modified date */
  WORD  ftime;      /* Last modified time */
  BYTE  fattrib;    /* Attribute */
  TCHAR  fname[13];    /* Short file name (8.3 format) */
#if _USE_LFN
  TCHAR*  lfname;      /* Pointer to the LFN buffer */
  UINT   lfsize;      /* Size of LFN buffer in TCHAR */
#endif
} FILINFO;

/* File function return code (FRESULT) */

typedef enum {
  FR_OK = 0,        /* (0) Succeeded */
  FR_DISK_ERR,      /* (1) A hard error occured in the low level disk I/O layer */
  FR_INT_ERR,        /* (2) Assertion failed */
  FR_NOT_READY,      /* (3) The physical drive cannot work */
  FR_NO_FILE,        /* (4) Could not find the file */
  FR_NO_PATH,        /* (5) Could not find the path */
  FR_INVALID_NAME,    /* (6) The path name format is invalid */
  FR_DENIED,        /* (7) Acces denied due to prohibited access or directory full */
  FR_EXIST,        /* (8) Acces denied due to prohibited access */
  FR_INVALID_OBJECT,    /* (9) The file/directory object is invalid */
  FR_WRITE_PROTECTED,    /* (10) The physical drive is write protected */
  FR_INVALID_DRIVE,    /* (11) The logical drive number is invalid */
  FR_NOT_ENABLED,      /* (12) The volume has no work area */
  FR_NO_FILESYSTEM,    /* (13) There is no valid FAT volume */
  FR_MKFS_ABORTED,    /* (14) The f_mkfs() aborted due to any parameter error */
  FR_TIMEOUT,        /* (15) Could not get a grant to access the volume within defined period */
  FR_LOCKED,        /* (16) The operation is rejected according to the file shareing policy */
  FR_NOT_ENOUGH_CORE,    /* (17) LFN working buffer could not be allocated */
  FR_TOO_MANY_OPEN_FILES,  /* (18) Number of open files > _FS_SHARE */
  FR_INVALID_PARAMETER  /* (19) Given parameter is invalid */
} FRESULT;

/*--------------------------------------------------------------*/
/* FatFs module application interface                           */

typedef FRESULT (* f_mount_t) (BYTE, FATFS*);            /* Mount/Unmount a logical drive */
#define f_mount(a,b) ((f_mount_t)(pgm_read_word(&_functable[_FT_f_mount])))((a),(b))
typedef FRESULT (* f_open_t) (FIL*, const TCHAR*, BYTE);      /* Open or create a file */
#define f_open(a,b,c) ((f_open_t)(pgm_read_word(&_functable[_FT_f_open])))((a),(b),(c))
typedef FRESULT (* f_read_t) (FIL*, void*, UINT, UINT*);      /* Read data from a file */
#define f_read(a,b,c,d) ((f_read_t)(pgm_read_word(&_functable[_FT_f_read])))((a),(b),(c),(d))
typedef FRESULT (* f_lseek_t) (FIL*, DWORD);            /* Move file pointer of a file object */
#define f_lseek(a,b) ((f_lseek_t)(pgm_read_word(&_functable[_FT_f_lseek])))((a),(b))
typedef FRESULT (* f_close_t) (FIL*);                /* Close an open file object */
#define f_close(a) ((f_close_t)(pgm_read_word(&_functable[_FT_f_close])))((a))
typedef FRESULT (* f_opendir_t) (DIR*, const TCHAR*);        /* Open an existing directory */
#define f_opendir(a,b) ((f_opendir_t)(pgm_read_word(&_functable[_FT_f_opendir])))((a),(b))
typedef FRESULT (* f_readdir_t) (DIR*, FILINFO*);          /* Read a directory item */
#define f_readdir(a,b) ((f_readdir_t)(pgm_read_word(&_functable[_FT_f_readdir])))((a),(b))
typedef FRESULT (* f_stat_t) (const TCHAR*, FILINFO*);      /* Get file status */
#define f_stat(a,b) ((f_stat_t)(pgm_read_word(&_functable[_FT_f_stat])))((a),(b))
typedef FRESULT (* f_write_t) (FIL*, const void*, UINT, UINT*);  /* Write data to a file */
#define f_write(a,b,c,d) ((f_write_t)(pgm_read_word(&_functable[_FT_f_write])))((a),(b),(c),(d))
typedef FRESULT (* f_getfree_t) (const TCHAR*, DWORD*, FATFS**);  /* Get number of free clusters on the drive */
#define f_getfree(a,b,c) ((f_getfree_t)(pgm_read_word(&_functable[_FT_f_getfree])))((a),(b),(c))
typedef FRESULT (* f_truncate_t) (FIL*);              /* Truncate file */
#define f_truncate(a) ((f_truncate_t)(pgm_read_word(&_functable[_FT_f_truncate])))((a))
typedef FRESULT (* f_sync_t) (FIL*);                /* Flush cached data of a writing file */
#define f_sync(a) ((f_sync_t)(pgm_read_word(&_functable[_FT_f_sync])))((a))
typedef FRESULT (* f_unlink_t) (const TCHAR*);          /* Delete an existing file or directory */
#define f_unlink(a) ((f_unlink_t)(pgm_read_word(&_functable[_FT_f_unlink])))((a))
typedef FRESULT  (* f_mkdir_t) (const TCHAR*);            /* Create a new directory */
#define f_mkdir(a) ((f_mkdir_t)(pgm_read_word(&_functable[_FT_f_mkdir])))((a))
typedef FRESULT (* f_chmod_t) (const TCHAR*, BYTE, BYTE);      /* Change attriburte of the file/dir */
#define f_chmod(a,b,c) ((f_chmod_t)(pgm_read_word(&_functable[_FT_f_chmod])))((a),(b),(c))
typedef FRESULT (* f_utime_t) (const TCHAR*, const FILINFO*);    /* Change timestamp of the file/dir */
#define f_utime(a,b) ((f_utime_t)(pgm_read_word(&_functable[_FT_f_utime])))((a),(b))
typedef FRESULT (* f_rename_t) (const TCHAR*, const TCHAR*);    /* Rename/Move a file or directory */
#define f_rename(a,b) ((f_rename_t)(pgm_read_word(&_functable[_FT_f_rename])))((a),(b))
typedef FRESULT (* f_chdrive_t) (BYTE);              /* Change current drive */
#define f_chdrive(a) ((f_chdrive_t)(pgm_read_word(&_functable[_FT_f_chdrive])))((a))
typedef FRESULT (* f_chdir_t) (const TCHAR*);            /* Change current directory */
#define f_chdir(a) ((f_chdir_t)(pgm_read_word(&_functable[_FT_f_chdir])))((a))
typedef FRESULT (* f_getcwd_t) (TCHAR*, UINT);          /* Get current directory */
#define f_getcwd(a,b) ((f_getcwd_t)(pgm_read_word(&_functable[_FT_f_getcwd])))((a),(b))
typedef FRESULT (* f_forward_t) (FIL*, UINT(*)(const BYTE*,UINT), UINT, UINT*);  /* Forward data to the stream */
#define f_forward(a,b,c,d) ((f_forward_t)(pgm_read_word(&_functable[_FT_f_forward])))((a),(b),(c),(d))
typedef FRESULT (* f_mkfs_t) (BYTE, BYTE, UINT);          /* Create a file system on the drive */
#define f_mkfs(a,b,c) ((f_mkfs_t)(pgm_read_word(&_functable[_FT_f_mkfs])))((a),(b),(c))

#define f_eof(fp) (((fp)->fptr == (fp)->fsize) ? 1 : 0)
#define f_error(fp) (((fp)->flag & FA__ERROR) ? 1 : 0)
#define f_tell(fp) ((fp)->fptr)
#define f_size(fp) ((fp)->fsize)

#ifndef EOF
#define EOF (-1)
#endif

/*--------------------------------------------------------------*/
/* Flags and offset address                                     */


/* File access control and file status flags (FIL.flag) */

#define  FA_READ           0x01
#define  FA_OPEN_EXISTING  0x00
#define  FA__ERROR         0x80

#if !_FS_READONLY
#define  FA_WRITE          0x02
#define  FA_CREATE_NEW     0x04
#define  FA_CREATE_ALWAYS  0x08
#define  FA_OPEN_ALWAYS    0x10
#define FA__WRITTEN        0x20
#define FA__DIRTY          0x40
#endif


/* FAT sub type (FATFS.fs_type) */

#define FS_FAT12  1
#define FS_FAT16  2
#define FS_FAT32  3


/* File attribute bits for directory entry */

#define  AM_RDO   0x01  /* Read only */
#define  AM_HID   0x02  /* Hidden */
#define  AM_SYS   0x04  /* System */
#define  AM_VOL   0x08  /* Volume label */
#define  AM_LFN   0x0F  /* LFN entry */
#define  AM_DIR   0x10  /* Directory */
#define  AM_ARC   0x20  /* Archive */
#define  AM_MASK  0x3F  /* Mask of defined bits */


/* Fast seek feature */
#define CREATE_LINKMAP  0xFFFFFFFF

#ifdef __cplusplus
}
#endif

// Determine whether or not an SD card is inserted
static inline uint8_t SOCKINS(void) { return !(PORTD.IN & _BV(3)); }

// Top-level function to initialize both the MMC and SD card drivers
extern void sd_init(uint8_t prescale);

#endif /* _FATFS */

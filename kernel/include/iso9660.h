/*
iso9660.h
ISO9660文件系统（CD）头文件
Copyright W24 Studio 
*/
#ifndef ISO9660_H
#define ISO9660_H
#include <stddef.h>
#include <vfs.h>

#define L9660_SEEK_END -1
#define L9660_SEEK_SET 0
#define L9660_SEEK_CUR +1

typedef enum {
	L9660_OK = 0,
	L9660_EIO,
	L9660_EBADFS,
	L9660_ENOENT,
	L9660_ENOTFILE,
	L9660_ENOTDIR,
} l9660_status;

typedef struct {
	uint8_t le[2];
} l9660_luint16;

typedef struct {
	uint8_t be[2];
} l9660_buint16;

typedef struct {
	uint8_t le[2], be[2];
} l9660_duint16;

typedef struct {
	uint8_t le[4];
} l9660_luint32;

typedef struct {
	uint8_t be[4];
} l9660_buint32;

typedef struct {
	uint8_t le[4], be[4];
} l9660_duint32;

typedef struct {
	char d[17];
} l9660_desctime;

typedef struct {
	char d[7];
} l9660_filetime;

typedef struct {
	uint8_t			length;
	uint8_t			xattr_length;
	l9660_duint32	sector;
	l9660_duint32	size;
	l9660_filetime	time;
	uint8_t			flags;
	uint8_t			unit_size;
	uint8_t			gap_size;
	l9660_duint16	vol_seq_number;
	uint8_t			name_len;
	char			name[/*name_len*/];
} l9660_dirent;

typedef struct {
	uint8_t type;
	char magic[5];
	uint8_t version;
} l9660_vdesc_header;

typedef struct {
	l9660_vdesc_header	hdr;
	char				pad0[1];
	char				system_id[32];
	char				volume_id[32];
	char				pad1[8];
	l9660_duint32		volume_space_size;
	char				pad2[32];
	l9660_duint16		volume_set_size;
	l9660_duint16		volume_seq_number;
	l9660_duint16		logical_block_size;
	l9660_duint32		path_table_size;
	l9660_luint32		path_table_le;
	l9660_luint32		path_table_opt_le;
	l9660_buint32		path_table_be;
	l9660_buint32		path_table_opt_be;
	union {
		l9660_dirent	root_dir_ent;
		char			pad3[34];
	};
	char				volume_set_id[128];
	char				data_preparer_id[128];
	char				app_id[128];
	char				copyright_file[38];
	char				abstract_file[36];
	char				bibliography_file[37];
	l9660_desctime		volume_created, volume_modified, volume_expires, volume_effective;
	uint8_t				file_structure_version;
	char				pad4[1];
	char				app_reserved[512];
	char				reserved[653];
} l9660_vdesc_primary;

typedef union {
	l9660_vdesc_header hdr;
	char _bits[2048];
} l9660_vdesc;

typedef struct l9660_fs {
#ifdef L9660_SINGLEBUFFER
	union {
		l9660_dirent root_dir_ent;
		char root_dir_pad[34];
	};
#else
	l9660_vdesc pvd;
#endif
	int (*read_sector)(struct l9660_fs *fs, void *buf, uint32_t sector);
	vfs_node_t device;
} l9660_fs;

typedef struct {
#ifndef L9660_SINGLEBUFFER
	char buf[2048];
#endif
	l9660_fs *fs;
	uint32_t first_sector;
	uint32_t position;
	uint32_t length;
} l9660_file;

typedef struct {
	l9660_file file;
} l9660_dir;

typedef struct l9660_fs_status {
	l9660_fs *fs;
	l9660_dir root_dir;
	l9660_dir now_dir;
} l9660_fs_status_t;

typedef struct file {
	int type;
	void *handle;
} *file_t;

void iso9660_regist(void);
#endif



#include <linux/fs.h>
#include <linux/vfs.h>
#include <linux/slab.h>

#include "squashfs_fs.h"
#include "squashfs_fs_sb.h"
#include "squashfs_fs_i.h"
#include "squashfs.h"

int squashfs_xattr_lookup(struct super_block *sb, unsigned int index,
		int *count, unsigned int *size, unsigned long long *xattr)
{
	struct squashfs_sb_info *msblk = sb->s_fs_info;
	int block = SQUASHFS_XATTR_BLOCK(index);
	int offset = SQUASHFS_XATTR_BLOCK_OFFSET(index);
	u64 start_block = le64_to_cpu(msblk->xattr_id_table[block]);
	struct squashfs_xattr_id id;
	int err;

	err = squashfs_read_metadata(sb, &id, &start_block, &offset,
							sizeof(id));
	if (err < 0)
		return err;

	*xattr = le64_to_cpu(id.xattr);
	*size = le32_to_cpu(id.size);
	*count = le32_to_cpu(id.count);
	return 0;
}


__le64 *squashfs_read_xattr_id_table(struct super_block *sb, u64 start,
		u64 *xattr_table_start, int *xattr_ids)
{
	unsigned int len;
	__le64 *xid_table;
	struct squashfs_xattr_id_table id_table;
	int err;

	err = squashfs_read_table(sb, &id_table, start, sizeof(id_table));
	if (err < 0) {
		ERROR("unable to read xattr id table\n");
		return ERR_PTR(err);
	}
	*xattr_table_start = le64_to_cpu(id_table.xattr_table_start);
	*xattr_ids = le32_to_cpu(id_table.xattr_ids);
	len = SQUASHFS_XATTR_BLOCK_BYTES(*xattr_ids);

	TRACE("In read_xattr_index_table, length %d\n", len);

	/* Allocate xattr id lookup table indexes */
	xid_table = kmalloc(len, GFP_KERNEL);
	if (xid_table == NULL) {
		ERROR("Failed to allocate xattr id index table\n");
		return ERR_PTR(-ENOMEM);
	}

	err = squashfs_read_table(sb, xid_table, start + sizeof(id_table), len);
	if (err < 0) {
		ERROR("unable to read xattr id index table\n");
		kfree(xid_table);
		return ERR_PTR(err);
	}

	return xid_table;
}

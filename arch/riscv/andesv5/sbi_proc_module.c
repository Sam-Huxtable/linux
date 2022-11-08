/*
 *  Copyright (C) 2009 Andes Technology Corporation
 *  Copyright (C) 2019 Andes Technology Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/blkdev.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <asm/andesv5/csr.h>
#include <asm/andesv5/proc.h>
#include <asm/sbi.h>

#define INPUTLEN 32

struct entry_struct{

	char *name;
	int perm;
	struct file_operations *fops;
};

static struct proc_dir_entry *proc_sbi;

#define DEBUG( enable, tagged, ...)				\
	do {							\
		if (enable) {					\
			if (tagged)				\
			printk("[ %30s() ] ", __func__);	\
			printk(__VA_ARGS__);			\
		}						\
	} while	(0)

static int debug = 0;
module_param(debug, int, 0);


static ssize_t proc_read_sbi_en(struct file *file, char __user *userbuf,
						size_t count, loff_t *ppos)
{
    int ret;
    char buf[18];
    if (!strncmp(file->f_path.dentry->d_name.name, "non_blocking", 12)) {
        ret = sprintf(buf, "non-blocking: %s\n", (get_non_blocking_status() & MMISC_CTL_NON_BLOCKING_ENABLE) ? "Enabled" : "Disabled");
    } else if (!strncmp(file->f_path.dentry->d_name.name, "write_around", 12)) {
        ret = sprintf(buf, "write_around: %s\n", (get_write_around_status() & MCACHE_CTL_DC_WAROUND_1_EN) ? "Enabled" : "Disabled");
    } else if (!strncmp(file->f_path.dentry->d_name.name, "l1i_prefetch", 12)) {
        ret = sprintf(buf, "l1i_prefetch: %s\n", (get_write_around_status() & MCACHE_CTL_L1I_PREFETCH_EN) ? "Enabled" : "Disabled");
    } else if (!strncmp(file->f_path.dentry->d_name.name, "l1d_prefetch", 12)) {
        ret = sprintf(buf, "l1d_prefetch: %s\n", (get_write_around_status() & MCACHE_CTL_L1D_PREFETCH_EN) ? "Enabled" : "Disabled");
    } else if (!strncmp(file->f_path.dentry->d_name.name, "mcache_ctl", 10)) {
        ret = sprintf(buf, "mcache_ctl: %lx\n", get_write_around_status());
    } else if (!strncmp(file->f_path.dentry->d_name.name, "mmisc_ctl", 9)) {
        ret = sprintf(buf, "mmisc_ctl: %lx\n", get_non_blocking_status());
    } else {
		return -EFAULT;
    }
    return simple_read_from_buffer(userbuf, count, ppos, buf, ret);
}

static ssize_t proc_write_sbi_en(struct file *file,
			const char __user *buffer, size_t count, loff_t *ppos)
{

	unsigned long en;
	char inbuf[INPUTLEN];

	if (count > INPUTLEN - 1)
		count = INPUTLEN - 1;

	if (copy_from_user(inbuf, buffer, count))
		return -EFAULT;

	inbuf[count] = '\0';

	if (!sscanf(inbuf, "%lx", &en))
		return -EFAULT;

	if (!strncmp(file->f_path.dentry->d_name.name, "non_blocking", 12)) {
		if (en && !(get_non_blocking_status() & MMISC_CTL_NON_BLOCKING_ENABLE)) {
			sbi_enable_non_blocking_load_store();
			DEBUG(debug, 1, "NON-blocking: Enabled\n");
		} else if (!en && (get_non_blocking_status() & MMISC_CTL_NON_BLOCKING_ENABLE)) {
			sbi_disable_non_blocking_load_store();
			DEBUG(debug, 1, "NON-blocking: Disabled\n");
		}
	} else if (!strncmp(file->f_path.dentry->d_name.name, "write_around", 12)) {
		if (en && !(get_write_around_status() & MCACHE_CTL_DC_WAROUND_1_EN)) {
			sbi_enable_write_around();
			DEBUG(debug, 1, "Write-around: Enabled\n");
		} else if (!en && (get_write_around_status() & MCACHE_CTL_DC_WAROUND_1_EN)) {
			sbi_disable_write_around();
			DEBUG(debug, 1, "Write-around: Disabled\n");
		}
	} else if (!strncmp(file->f_path.dentry->d_name.name, "l1i_prefetch", 12)) {
		if (en && !(get_write_around_status() & MCACHE_CTL_L1I_PREFETCH_EN)) {
			sbi_enable_l1i_cache();
			DEBUG(debug, 1, "L1I_cache_Prefetch: Enabled\n");
		} else if (!en && (get_write_around_status() & MCACHE_CTL_L1I_PREFETCH_EN)) {
			sbi_disable_l1i_cache();
			DEBUG(debug, 1, "L1I_cache_Prefetch: Disabled\n");
		}
	} else if (!strncmp(file->f_path.dentry->d_name.name, "l1d_prefetch", 12)) {
		if (en && !(get_write_around_status() & MCACHE_CTL_L1D_PREFETCH_EN)) {
			sbi_enable_l1d_cache();
			DEBUG(debug, 1, "L1D_cache_Prefetch: Enabled\n");
		} else if (!en && (get_write_around_status() & MCACHE_CTL_L1D_PREFETCH_EN)) {
			sbi_disable_l1d_cache();
			DEBUG(debug, 1, "L1D_cache_Prefetch: Disabled\n");
		}
	} else if (!strncmp(file->f_path.dentry->d_name.name, "mcache_ctl", 10)) {
		sbi_set_mcache_ctl(en);
	} else if (!strncmp(file->f_path.dentry->d_name.name, "mmisc_ctl", 9)) {
		sbi_set_mmisc_ctl(en);
	} else
		return -EFAULT;

	return count;
}

static struct file_operations en_fops = {
	.open = simple_open,
	.read = proc_read_sbi_en,
	.write = proc_write_sbi_en,
};

static void create_seq_entry(struct entry_struct *e, mode_t mode,
			     struct proc_dir_entry *parent)
{

	struct proc_dir_entry *entry = proc_create(e->name, mode, parent, e->fops);

	if (!entry)
		printk(KERN_ERR "invalid %s register.\n", e->name);
}

static void install_proc_table(struct entry_struct *table)
{
	while (table->name) {

		create_seq_entry(table, table->perm, proc_sbi);
		table++;
	}
}

static void remove_proc_table(struct entry_struct *table)
{

	while (table->name) {
		remove_proc_entry(table->name, proc_sbi);
		table++;
	}
}

struct entry_struct proc_table_sbi[] = {

	{"non_blocking", 0644, &en_fops},	//sbi_ae350_non_blocking_load_store
	{"write_around", 0644, &en_fops},       //sbi_ae350_write_around
	{"l1i_prefetch", 0644, &en_fops},       //sbi_ae350_L1I-prefetch
	{"l1d_prefetch", 0644, &en_fops},       //sbi_ae350_L1D-prefetch
	{"mcache_ctl", 0644, &en_fops},		//sbi_ae350_mcache_ctl
	{"mmisc_ctl", 0644, &en_fops},		//sbi_ae350_mmisc_ctl
};
static int __init init_sbi(void)
{

	DEBUG(debug, 0, "SBI module registered\n");

	if (!(proc_sbi = proc_mkdir("sbi", NULL)))
		return -ENOMEM;

	install_proc_table(proc_table_sbi);

	return 0;
}

static void __exit cleanup_sbi(void)
{

	remove_proc_table(proc_table_sbi);
	remove_proc_entry("sbi", NULL);

	DEBUG(debug, 1, "SBI module unregistered\n");
}

module_init(init_sbi);
module_exit(cleanup_sbi);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Userspace SBI Module");

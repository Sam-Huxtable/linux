// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 AllianceMemory.
 *
 */

#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/mtd/spinand.h>							  
#define SPINAND_MFR_ALLIANCE		0x52

#define STATUS_ECC_LIMIT_BITFLIPS (3 << 4)

static SPINAND_OP_VARIANTS(read_cache_variants,
		SPINAND_PAGE_READ_FROM_CACHE_QUADIO_OP(0, 2, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_X4_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_DUALIO_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_X2_OP(0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_OP(true, 0, 1, NULL, 0),
		SPINAND_PAGE_READ_FROM_CACHE_OP(false, 0, 1, NULL, 0));

static SPINAND_OP_VARIANTS(write_cache_variants,
		SPINAND_PROG_LOAD_X4(true, 0, NULL, 0),
		SPINAND_PROG_LOAD(true, 0, NULL, 0));

static SPINAND_OP_VARIANTS(update_cache_variants,
		SPINAND_PROG_LOAD_X4(false, 0, NULL, 0),
		SPINAND_PROG_LOAD(false, 0, NULL, 0));

static int alliance_ooblayout_ecc(struct mtd_info *mtd, int section,
				  struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	region->offset = mtd->oobsize / 2;
	region->length = mtd->oobsize / 2;

	return 0;
}

static int alliance_ooblayout_free(struct mtd_info *mtd, int section,
				   struct mtd_oob_region *region)
{
	if (section)
		return -ERANGE;

	/* Reserve 1 bytes for the BBM. */
	region->offset = 1;
	region->length = (mtd->oobsize / 2) - 1;

	return 0;
}

static const struct mtd_ooblayout_ops alliance_ooblayout = {
	.ecc = alliance_ooblayout_ecc,
	.free = alliance_ooblayout_free,
};

static int alliance_ecc_get_status(struct spinand_device *spinand,
				   u8 status)
{
	switch (status & STATUS_ECC_MASK) {
	case STATUS_ECC_NO_BITFLIPS:
		return 0;

	case STATUS_ECC_UNCOR_ERROR:
		return -EBADMSG;

	case STATUS_ECC_HAS_BITFLIPS:
		return 1;

	case STATUS_ECC_LIMIT_BITFLIPS:
		return 3;


	default:
		break;
	}

	return -EINVAL;
}

static const struct spinand_info alliance_spinand_table[] = {		     
	SPINAND_INFO("AS5F18G04SND-10LIN", 
		     SPINAND_ID(SPINAND_READID_METHOD_OPCODE_ADDR, 0x8D),
		     NAND_MEMORG(1, 4096, 256, 64, 4096, 80, 1, 1, 1),
		     NAND_ECCREQ(8, 512),
		     SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					      &write_cache_variants,
					      &update_cache_variants),
		     0,
		     SPINAND_ECCINFO(&alliance_ooblayout, alliance_ecc_get_status)),
};

static int alliance_spianand_detect(struct spinand_device *spinand)
{
	u8 *id = spinand->id.data;
	int ret;
	
	/*
	 * Macronix SPI NAND read ID needs a dummy byte, so the first byte in
	 * raw_id is garbage.
	 */
	if (id[0] != SPINAND_MFR_ALLIANCE)
		return 0;
	
	ret = spinand_match_and_init(spinand, alliance_spinand_table,
				     ARRAY_SIZE(alliance_spinand_table),
				     id[1]);
	
	if (ret)
		return ret;
	
	return 1;
}

static const struct spinand_manufacturer_ops alliance_spinand_manuf_ops = {
};

const struct spinand_manufacturer alliance_spinand_manufacturer = {
	.id = SPINAND_MFR_ALLIANCE,
	.name = "Alliance",
 	.chips = alliance_spinand_table,
  	.nchips = ARRAY_SIZE(alliance_spinand_table),
	.ops = &alliance_spinand_manuf_ops,
};

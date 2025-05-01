
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/mtd/spinand.h>							  

#define SPINAND_MFR_issi			0x9D

#define IS37SMW04G8B_STATUS_ECC_1_7_BITFLIPS  (1 << 4)
#define IS37SMW04G8B_STATUS_ECC_8_BITFLIPS    (3 << 4)

#define IS37SMW04G8B_REG_STATUS2           0xf0



#define AM_STATUS_ECC_BITMASK       (7 << 4)  // 0x70, covering ECCS2:ECCS0
#define AM_STATUS_ECC_NONE_DETECTED (0 << 4)  // 0x00
#define AM_STATUS_ECC_CORRECTED     (1 << 4)  // 0x10 (1-3 bit errors corrected)
#define AM_STATUS_ECC_UNCORRECTABLE (2 << 4)  // 0x20 (More than 8 bit errors)
#define AM_STATUS_ECC_REFRESH_REC   (3 << 4)  // 0x30 (4-6 bit errors, refresh recommended)
#define AM_STATUS_ECC_RESERVED_4    (4 << 4)  // 0x40 (Reserved)
#define AM_STATUS_ECC_REFRESH_MAND  (5 << 4)  // 0x50 (7-8 bit errors, refresh mandatory)
#define AM_STATUS_ECC_RESERVED_6    (6 << 4)  // 0x60 (Reserved)
#define AM_STATUS_ECC_INVALID       (7 << 4)  // 0x70 (Invalid state)

static SPINAND_OP_VARIANTS(read_cache_variants,
                    			SPINAND_PAGE_READ_FROM_CACHE_QUADIO_OP(0, 1, NULL, 0),
                    			SPINAND_PAGE_READ_FROM_CACHE_X4_OP(0, 1, NULL, 0),
                    			SPINAND_PAGE_READ_FROM_CACHE_DUALIO_OP(0, 1, NULL, 0),
                    			SPINAND_PAGE_READ_FROM_CACHE_X2_OP(0, 1, NULL, 0),
                    			SPINAND_PAGE_READ_FROM_CACHE_OP(true, 0, 1, NULL, 0),
                    			SPINAND_PAGE_READ_FROM_CACHE_OP(false, 0, 1, NULL, 0));

static SPINAND_OP_VARIANTS(write_cache_variants,
			                     SPINAND_PROG_LOAD_X4(false,0, NULL, 0),
			                     SPINAND_PROG_LOAD(false, 0, NULL, 0));

static SPINAND_OP_VARIANTS(update_cache_variants,
                          SPINAND_PROG_LOAD_X4(false, 0, NULL, 0),
                          SPINAND_PROG_LOAD(false, 0, NULL, 0));

static int Issi_ooblayout_ecc(struct mtd_info *mtd, int section,
				  struct mtd_oob_region *region)
{
    if (section > 0)
        return -ERANGE;

    region->offset = 64;    // ECC occupies upper half of OOB
    region->length = 64;
    return 0;
}

static int Issi_ooblayout_free(struct mtd_info *mtd, int section,
				   struct mtd_oob_region *region)
{
    if (section > 0)
        return -ERANGE;

    region->offset = 0;     // Free area is in the first half of OOB
    region->length = 64;
    return 0;
}

static const struct mtd_ooblayout_ops Issi_ooblayout = {
	.ecc = Issi_ooblayout_ecc,
	.free = Issi_ooblayout_free,
};

static int IS37SMW04G8B_ecc_get_status(struct spinand_device *spinand, uint8_t status)
{
        
	switch (status & AM_STATUS_ECC_BITMASK) {
        case AM_STATUS_ECC_NONE_DETECTED:
            return 0;
    
        case AM_STATUS_ECC_CORRECTED:
    
                return 3;
    
        default:
            return -2;;
        }
    
        return -1;
}

static const struct spinand_info issi_spinand_table[] = {
    SPINAND_INFO("IS37SMW04G8B", 
             SPINAND_ID(SPINAND_READID_METHOD_OPCODE_DUMMY, 0x35),
             NAND_MEMORG(1, 2048, 128, 64, 2048, 40, 0, 2, 1),
             NAND_ECCREQ(8, 544),
		         SPINAND_INFO_OP_VARIANTS(&read_cache_variants,
					                            &write_cache_variants,
					                            &update_cache_variants),
            SPINAND_HAS_QE_BIT,
             SPINAND_ECCINFO(&Issi_ooblayout, IS37SMW04G8B_ecc_get_status)),
};


static const struct spinand_manufacturer_ops issi_spinand_manuf_ops = {
};

const struct spinand_manufacturer issi_spinand_manufacturer = {
    .id = SPINAND_MFR_issi,
    .name = "issi",
    .chips = issi_spinand_table,
    .nchips = ARRAY_SIZE(issi_spinand_table),
    .ops = &issi_spinand_manuf_ops,
};

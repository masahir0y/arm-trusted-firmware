
#include <arch_helpers.h>
#include <io/io_block.h>
#include <mmio.h>
#include <plat_uniphier.h>
#include <platform_def.h>
#include <sys/types.h>
#include <utils.h>

#define UNIPHIER_ROM_BUFFER_BASE	(BL31_LIMIT)
#define UNIPHIER_ROM_BUFFER_SIZE	(SZ_4M)

#define UNIPHIER_EMMC_SOFTWARE_RESET	0x5a00022f

static int (*uniphier_rom_emmc_load_image)(uint32_t block_or_byte, uintptr_t buf,
					  uint32_t block_cnt);
static int uniphier_rom_emmc_is_over_2gb;

struct uniphier_rom_func_table {
	unsigned int soc_id;
	uintptr_t emmc_is_over_2gb;
	uintptr_t emmc_set_part_access;
	uintptr_t emmc_clear_part_access;
	uintptr_t emmc_send_cmd;
	uintptr_t emmc_load_image;
	uintptr_t nand_load_image;
	uintptr_t usb_load_image;
};

static const struct uniphier_rom_func_table uniphier_rom_func_table[] = {
	{
		.soc_id = UNIPHIER_LD11_ID,
		.emmc_is_over_2gb = 0x1b68,
		.emmc_set_part_access = 0x1c38,
		.emmc_clear_part_access = 0x1cd0,
		.emmc_send_cmd = 0x20d8,
		.emmc_load_image = 0x2e48,
		.nand_load_image = 0x34f0,
		.usb_load_image = 0x3958,
	},
	{
		.soc_id = UNIPHIER_LD20_ID,
		.emmc_is_over_2gb = 0x1ba0,
		.emmc_set_part_access = 0x1c70,
		.emmc_clear_part_access = 0x1d10,
		.emmc_send_cmd = 0x2130,
		.emmc_load_image = 0x2ef0,
		.nand_load_image = 0x35d0,
		.usb_load_image = 0x37f0,
	},
};

static const struct uniphier_rom_func_table *uniphier_get_rom_func_table(
								unsigned int id)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(uniphier_rom_func_table); i++) {
		if (uniphier_rom_func_table[i].soc_id == id)
			return &uniphier_rom_func_table[i];
	}

	return NULL;
}

static size_t uniphier_rom_emmc_read(int lba, uintptr_t buf, size_t size)
{
	int ret;

	inv_dcache_range(buf, size);

	if (!uniphier_rom_emmc_is_over_2gb)
		lba *= 512;

	ret = uniphier_rom_emmc_load_image(lba, buf, size / 512);

	inv_dcache_range(buf, size);

	return ret ? 0 : size;
}

static const struct io_block_dev_spec uniphier_rom_emmc_dev_spec = {
	.buffer = {
		.offset = UNIPHIER_ROM_BUFFER_BASE,
		.length = UNIPHIER_ROM_BUFFER_SIZE,
	},
	.ops = {
		.read = uniphier_rom_emmc_read,
	},
	.block_size = 512,
};

#define UNIPHIER_ROM_SET_PTR(val, ptr)	(val = (__typeof(val))(ptr))

static int __uniphier_rom_emmc_init(const struct uniphier_rom_func_table *funcs)
{
	const uint32_t rca = 0x1000; /* RCA assigned by Boot ROM */
	int (*send_cmd)(uint32_t cmd, uint32_t arg);
	int (*is_over_2gb)(uint32_t rca);
	int (*set_part_access)(int part_num);
	int (*clear_part_access)(int part_num);
	int ret;

	UNIPHIER_ROM_SET_PTR(send_cmd, funcs->emmc_send_cmd);
	UNIPHIER_ROM_SET_PTR(is_over_2gb, funcs->emmc_is_over_2gb);
	UNIPHIER_ROM_SET_PTR(set_part_access, funcs->emmc_set_part_access);
	UNIPHIER_ROM_SET_PTR(clear_part_access, funcs->emmc_clear_part_access);
	UNIPHIER_ROM_SET_PTR(uniphier_rom_emmc_load_image,
			     funcs->emmc_load_image);

	/*
	 * deselect card before SEND_CSD command.
	 * Do not check the return code.  It fails, but it is OK.
	 */
	send_cmd(0x071a0000, 0); /* CMD7 (arg=0) */

	/* reset CMD Line */
	mmio_write_8(UNIPHIER_EMMC_SOFTWARE_RESET, 0x6);
	while (mmio_read_8(UNIPHIER_EMMC_SOFTWARE_RESET))
		;

	uniphier_rom_emmc_is_over_2gb = is_over_2gb(rca);

	ret = send_cmd(0x071a0000, rca << 16); /* CMD7: select again */
	if (ret)
		return -EIO;

	ret = clear_part_access(7);
	if (ret)
		return -EIO;

	ret = set_part_access(1);	/* Switch to Boot Partition 1 */
	if (ret)
		return -EIO;

	return 0;
}

int uniphier_rom_emmc_init(unsigned int soc_id, uintptr_t *block_dev_spec)
{
	const struct uniphier_rom_func_table *funcs;
	int ret;

	funcs = uniphier_get_rom_func_table(soc_id);
	if (!funcs)
		return -EINVAL;

	ret = __uniphier_rom_emmc_init(funcs);
	if (ret)
		return ret;

	*block_dev_spec = (uintptr_t)&uniphier_rom_emmc_dev_spec;

	return 0;
}

static size_t uniphier_rom_nand_read(int lba, uintptr_t buf, size_t size)
{
	return 0;
}

static const struct io_block_dev_spec uniphier_rom_nand_dev_spec = {
	.buffer = {
		.offset = UNIPHIER_ROM_BUFFER_BASE,
		.length = UNIPHIER_ROM_BUFFER_SIZE,
	},
	.ops = {
		.read = uniphier_rom_nand_read,
	},
	.block_size = 512,
};

int uniphier_rom_nand_init(unsigned int soc_id, uintptr_t *block_dev_spec)
{
	const struct uniphier_rom_func_table *funcs;

	funcs = uniphier_get_rom_func_table(soc_id);
	if (!funcs)
		return -EINVAL;

	*block_dev_spec = (uintptr_t)&uniphier_rom_nand_dev_spec;

	return 0;
}

static size_t uniphier_rom_usb_read(int lba, uintptr_t buf, size_t size)
{
	return 0;
}

static const struct io_block_dev_spec uniphier_rom_usb_dev_spec = {
	.buffer = {
		.offset = UNIPHIER_ROM_BUFFER_BASE,
		.length = UNIPHIER_ROM_BUFFER_SIZE,
	},
	.ops = {
		.read = uniphier_rom_usb_read,
	},
	.block_size = 512,
};

int uniphier_rom_usb_init(unsigned int soc_id, uintptr_t *block_dev_spec)
{
	const struct uniphier_rom_func_table *funcs;

	funcs = uniphier_get_rom_func_table(soc_id);
	if (!funcs)
		return -EINVAL;

	*block_dev_spec = (uintptr_t)&uniphier_rom_usb_dev_spec;

	return 0;
}

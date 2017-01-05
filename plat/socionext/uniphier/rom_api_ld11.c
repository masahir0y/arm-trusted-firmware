#include "rom_api.h"

const struct uniphier_rom_func_table uniphier_rom_func_table = {
	.emmc_is_over_2gb = 0x1b68,
	.emmc_set_part_access = 0x1c38,
	.emmc_clear_part_access = 0x1cd0,
	.emmc_send_cmd = 0x20d8,
	.emmc_load_image = 0x2e48,
	.nand_load_image = 0x34f0,
	.usb_load_image = 0x3958,
};

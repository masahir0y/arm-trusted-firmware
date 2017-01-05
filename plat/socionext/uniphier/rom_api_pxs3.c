#include "rom_api.h"

const struct uniphier_rom_func_table uniphier_rom_func_table = {
	.emmc_is_over_2gb = 0x1ba0,
	.emmc_set_part_access = 0x1c70,
	.emmc_clear_part_access = 0x1d10,
	.emmc_send_cmd = 0x2130,
	.emmc_load_image = 0x2ef0,
	.nand_load_image = 0x35d0,
	.usb_load_image = 0x37f0,
};

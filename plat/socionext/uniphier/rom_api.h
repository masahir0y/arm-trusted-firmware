#ifndef __ROM_API_H__
#define __ROM_API_H__

#include <sys/types.h>

struct uniphier_rom_func_table {
	uintptr_t emmc_is_over_2gb;
	uintptr_t emmc_set_part_access;
	uintptr_t emmc_clear_part_access;
	uintptr_t emmc_send_cmd;
	uintptr_t emmc_load_image;
	uintptr_t nand_load_image;
	uintptr_t usb_load_image;
};

extern const struct uniphier_rom_func_table uniphier_rom_func_table;

#endif /* __ROM_API_H__ */

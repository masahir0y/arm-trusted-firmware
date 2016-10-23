
#include <mmio.h>
#include <plat_uniphier.h>
#include <stddef.h>
#include <utils.h>

#define UNIPHIER_PINMON0		0x5f900100

static int uniphier_ld11_is_usb_boot(uint32_t pinmon)
{
	return !!(~pinmon & 0x00000080);
}

static int uniphier_ld20_is_usb_boot(uint32_t pinmon)
{
	return !!(~pinmon & 0x00000780);
}

static const int uniphier_boot_device_table[] = {
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_NAND,
	UNIPHIER_BOOT_DEVICE_EMMC,
	UNIPHIER_BOOT_DEVICE_EMMC,
	UNIPHIER_BOOT_DEVICE_EMMC,
	UNIPHIER_BOOT_DEVICE_EMMC,
	UNIPHIER_BOOT_DEVICE_EMMC,
	UNIPHIER_BOOT_DEVICE_EMMC,
	UNIPHIER_BOOT_DEVICE_EMMC,
	UNIPHIER_BOOT_DEVICE_NOR,
};

struct uniphier_boot_device_info {
	unsigned int soc_id;
	int (*is_usb_boot)(uint32_t pinmon);
	const int *boot_device_table;
};

static const struct uniphier_boot_device_info uniphier_boot_device_info[] = {
	{
		.soc_id = UNIPHIER_LD11_ID,
		.is_usb_boot = uniphier_ld11_is_usb_boot,
		.boot_device_table = uniphier_boot_device_table,
	},
	{
		.soc_id = UNIPHIER_LD20_ID,
		.is_usb_boot = uniphier_ld20_is_usb_boot,
		.boot_device_table = uniphier_boot_device_table,
	},
};

static const struct uniphier_boot_device_info *uniphier_get_boot_device_info(
								unsigned int id)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(uniphier_boot_device_info); i++) {
		if (uniphier_boot_device_info[i].soc_id == id)
			return &uniphier_boot_device_info[i];
	}

	return NULL;
}

int uniphier_get_boot_device(unsigned int soc_id)
{
	const struct uniphier_boot_device_info *info;
	uint32_t pinmon;

	info = uniphier_get_boot_device_info(soc_id);
	if (!info)
		return -EINVAL;

	pinmon = mmio_read_32(UNIPHIER_PINMON0);

	if (info->is_usb_boot(pinmon))
		return UNIPHIER_BOOT_DEVICE_USB;

	return info->boot_device_table[(pinmon >> 1) & 0x1f];
}

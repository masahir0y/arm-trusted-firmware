#include "boot_device.h"
#include "uniphier.h"

static int is_usb_boot(uint32_t pinmon)
{
	return !!(~pinmon & 0x00000080);
}

static const int boot_device_table[] = {
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

const struct uniphier_boot_device_info uniphier_boot_device_info = {
	.is_usb_boot = is_usb_boot,
	.boot_device_table = boot_device_table,
};

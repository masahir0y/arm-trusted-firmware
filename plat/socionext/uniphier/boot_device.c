
#include <mmio.h>

#include "boot_device.h"
#include "uniphier.h"

#define UNIPHIER_PINMON0		0x5f900100

int uniphier_get_boot_device(void)
{
	const struct uniphier_boot_device_info *info;
	uint32_t pinmon;

	info = &uniphier_boot_device_info;
	pinmon = mmio_read_32(UNIPHIER_PINMON0);

	if (info->is_usb_boot(pinmon))
		return UNIPHIER_BOOT_DEVICE_USB;

	return info->boot_device_table[(pinmon >> 1) & 0x1f];
}

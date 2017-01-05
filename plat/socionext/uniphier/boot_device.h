#ifndef __BOOT_DEVICE_H__
#define __BOOT_DEVICE_H__

#include <sys/types.h>

struct uniphier_boot_device_info {
	int (*is_usb_boot)(uint32_t pinmon);
	const int *boot_device_table;
};

extern const struct uniphier_boot_device_info uniphier_boot_device_info;

#endif /* __BOOT_DEVICE_H__ */

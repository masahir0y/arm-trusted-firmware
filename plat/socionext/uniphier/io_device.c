
#include <assert.h>
#include <firmware_image_package.h>
#include <io/io_block.h>
#include <io/io_driver.h>
#include <io/io_fip.h>
#include <io/io_memmap.h>
#include <platform_def.h>
#include <types.h>
#include <utils.h>

#include "uniphier.h"

static const io_dev_connector_t *uniphier_fip_dev_con;
static uintptr_t uniphier_fip_dev_handle;

static const io_dev_connector_t *uniphier_backend_dev_con;
static uintptr_t uniphier_backend_dev_handle;

static io_block_spec_t uniphier_fip_spec = {
	/* .offset will be set by the io_setup func */
	.length = SZ_2M,
};

static const io_uuid_spec_t uniphier_bl2_spec = {
	.uuid = UUID_TRUSTED_BOOT_FIRMWARE_BL2,
};

static const io_uuid_spec_t uniphier_bl31_spec = {
	.uuid = UUID_EL3_RUNTIME_FIRMWARE_BL31,
};

static const io_uuid_spec_t uniphier_bl32_spec = {
	.uuid = UUID_SECURE_PAYLOAD_BL32,
};

static const io_uuid_spec_t uniphier_bl33_spec = {
	.uuid = UUID_NON_TRUSTED_FIRMWARE_BL33,
};

struct uniphier_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	uintptr_t init_params;
};

static const struct uniphier_io_policy uniphier_io_policies[] = {
	[FIP_IMAGE_ID] = {
		.dev_handle = &uniphier_backend_dev_handle,
		.image_spec = (uintptr_t)&uniphier_fip_spec,
	},
	[BL2_IMAGE_ID] = {
		.dev_handle = &uniphier_fip_dev_handle,
		.image_spec = (uintptr_t)&uniphier_bl2_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[BL31_IMAGE_ID] = {
		.dev_handle = &uniphier_fip_dev_handle,
		.image_spec = (uintptr_t)&uniphier_bl31_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[BL32_IMAGE_ID] = {
		.dev_handle = &uniphier_fip_dev_handle,
		.image_spec = (uintptr_t)&uniphier_bl32_spec,
		.init_params = FIP_IMAGE_ID,
	},
	[BL33_IMAGE_ID] = {
		.dev_handle = &uniphier_fip_dev_handle,
		.image_spec = (uintptr_t)&uniphier_bl33_spec,
		.init_params = FIP_IMAGE_ID,
	},
};

static int uniphier_io_block_setup(size_t fip_offset, uintptr_t block_dev_spec)
{
	int ret;

	uniphier_fip_spec.offset = fip_offset;

	ret = register_io_dev_block(&uniphier_backend_dev_con);
	if (ret)
		return ret;
	
	return io_dev_open(uniphier_backend_dev_con, block_dev_spec,
			   &uniphier_backend_dev_handle);
}

static int uniphier_io_memmap_setup(size_t fip_offset)
{
	int ret;

	uniphier_fip_spec.offset = fip_offset;

	ret = register_io_dev_memmap(&uniphier_backend_dev_con);
	if (ret)
		return ret;

	return io_dev_open(uniphier_backend_dev_con, 0,
			   &uniphier_backend_dev_handle);
}

static int uniphier_io_fip_setup(void)
{
	int ret;

	ret = register_io_dev_fip(&uniphier_fip_dev_con);
	if (ret)
		return ret;

	return io_dev_open(uniphier_fip_dev_con, 0, &uniphier_fip_dev_handle);
}

static int uniphier_io_emmc_setup(void)
{
	uintptr_t block_dev_spec;
	int ret;

	ret = uniphier_rom_emmc_init(&block_dev_spec);
	if (ret)
		return ret;

	return uniphier_io_block_setup(0x20000, block_dev_spec);
}

static int uniphier_io_nand_setup(void)
{
	uintptr_t block_dev_spec;
	int ret;

	ret = uniphier_rom_nand_init(&block_dev_spec);
	if (ret)
		return ret;

	return uniphier_io_block_setup(0x20000, block_dev_spec);
}

static int uniphier_io_nor_setup(void)
{
	return uniphier_io_memmap_setup(0x80000);
}

static int uniphier_io_usb_setup(void)
{
	uintptr_t block_dev_spec;
	int ret;

	ret = uniphier_rom_usb_init(&block_dev_spec);
	if (ret)
		return ret;

	return uniphier_io_block_setup(0x20000, block_dev_spec);
}

static int (* const uniphier_io_setup_table[])(void) = {
	[UNIPHIER_BOOT_DEVICE_EMMC] = uniphier_io_emmc_setup,
	[UNIPHIER_BOOT_DEVICE_NAND] = uniphier_io_nand_setup,
	[UNIPHIER_BOOT_DEVICE_NOR] = uniphier_io_nor_setup,
	[UNIPHIER_BOOT_DEVICE_USB] = uniphier_io_usb_setup,
};

int uniphier_io_setup(void)
{
	int (*io_setup)(void);
	int boot_dev;
	int ret;

	boot_dev = uniphier_get_boot_device();
	if (boot_dev < 0)
		return boot_dev;

	io_setup = uniphier_io_setup_table[boot_dev];
	ret = io_setup();
	if (ret)
		return ret;

	ret = uniphier_io_fip_setup();
	if (ret)
		return ret;

	return 0;
}

int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	uintptr_t init_params;

	assert(image_id < ARRAY_SIZE(uniphier_io_policies));

	*dev_handle = *(uniphier_io_policies[image_id].dev_handle);
	*image_spec = uniphier_io_policies[image_id].image_spec;
	init_params = uniphier_io_policies[image_id].init_params;

	return io_dev_init(*dev_handle, init_params);
}

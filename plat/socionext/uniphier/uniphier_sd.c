/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <stdint.h>

#include <platform_def.h>

#include <arch_helpers.h>
#include <drivers/io/io_block.h>
#include <lib/utils_def.h>

#include "uniphier.h"

static int (*__uniphier_sd_read)(int lba, uintptr_t buf, size_t size);

static void uniphier_nx1_sd_init(void)
{
	/* Nothing */
}

static int uniphier_nx1_sd_read(int lba, uintptr_t buf, size_t size)
{
	static int (*rom_sd_read)(unsigned int lba, uintptr_t buf,
				  unsigned int size);
	int ret;

	rom_sd_read = (__typeof(rom_sd_read))0x1008;

	/* ROM-API - return 0 on success, 1 on error */
	ret = rom_sd_read(lba, buf, size);

	return ret ? -1 : 0;
}

struct uniphier_sd_rom_param {
	void (*init)(void);
	int (*read)(int lba, uintptr_t buf, size_t size);
};

static const struct uniphier_sd_rom_param uniphier_sd_rom_params[] = {
	[UNIPHIER_SOC_NX1] = {
		.init = uniphier_nx1_sd_init,
		.read = uniphier_nx1_sd_read,
	},
};

static size_t uniphier_sd_read(int lba, uintptr_t buf, size_t size)
{
	int ret;

	inv_dcache_range(buf, size);

	ret = __uniphier_sd_read(lba, buf, size / 512);

	inv_dcache_range(buf, size);

	return ret ? 0 : size;
}

static struct io_block_dev_spec uniphier_sd_dev_spec = {
	.ops = {
		.read = uniphier_sd_read,
	},
	.block_size = 512,
};

int uniphier_sd_init(unsigned int soc,
		     struct io_block_dev_spec **block_dev_spec)
{
	const struct uniphier_sd_rom_param *param;

	assert(soc < ARRAY_SIZE(uniphier_sd_rom_params));
	param = &uniphier_sd_rom_params[soc];

	if (param->init)
		param->init();

	__uniphier_sd_read = param->read;

	*block_dev_spec = &uniphier_sd_dev_spec;

	return 0;
}

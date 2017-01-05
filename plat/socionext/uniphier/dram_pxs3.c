/*
 * Copyright (C) 2017 Socionext Inc.
 */

#include <debug.h>
#include <mmio.h>

#include "dram_setup.h"
#include "umc_regs.h"
#include "uniphier.h"

static int dram_init(const struct uniphier_dram_data *dram)
{
	return 0;
}

struct uniphier_dram_setup_info uniphier_dram_setup_info = {
	.dram_init = dram_init,
	.memconf_init = uniphier_memconf_3ch_init,
	.rst_deassert_bits = 0x07010707,
	.clk_enable_bits = 0x00010007,
};

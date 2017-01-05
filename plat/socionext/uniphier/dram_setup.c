#include <debug.h>
#include <mmio.h>

#include "dram_setup.h"
#include "uniphier.h"

#define UNIPHIER_RSTCTRL7		0x61842018
#define UNIPHIER_CLKCTRL7		0x61842118

int uniphier_dram_setup(const struct uniphier_dram_data *dram)
{
	struct uniphier_dram_setup_info *info = &uniphier_dram_setup_info;
	int ret;

	ret = info->memconf_init(dram);
	if (ret) {
		ERROR("failed to init MEMCONF (%d)\n", ret);
		return ret;
	}

	mmio_setbits_32(UNIPHIER_RSTCTRL7, info->rst_deassert_bits);
	mmio_setbits_32(UNIPHIER_CLKCTRL7, info->clk_enable_bits);

	ret = info->dram_init(dram);
	if (ret) {
		ERROR("failed to init DRAM (%d)\n", ret);
		return ret;
	}

	return 0;
}

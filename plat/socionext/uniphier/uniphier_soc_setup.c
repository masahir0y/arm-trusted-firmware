
#include <debug.h>
#include <mmio.h>
#include <plat_uniphier.h>
#include <utils.h>

struct uniphier_initdata {
	int (*memconf_init)(const struct uniphier_dram_data *dram);
	void (*clk_enable)(void);
	int (*umc_init)(const struct uniphier_dram_data *dram);
};

static const struct uniphier_initdata uniphier_initdata[] = {
	[UNIPHIER_SOC_LD11] = {
		.memconf_init = uniphier_memconf_2ch_init,
		.clk_enable = uniphier_ld11_clk_enable_dram,
		.umc_init = uniphier_ld11_umc_init,
	},
	[UNIPHIER_SOC_LD20] = {
		.memconf_init = uniphier_memconf_3ch_init,
		.clk_enable = uniphier_ld20_clk_enable_dram,
		.umc_init = uniphier_ld20_umc_init,
	},
	[UNIPHIER_SOC_PXS3] = {
		.memconf_init = uniphier_memconf_3ch_init,
		.clk_enable = uniphier_pxs3_clk_enable_dram,
		.umc_init = uniphier_pxs3_umc_init,
	},
};

int uniphier_soc_setup(unsigned int soc, const struct uniphier_dram_data *dram)
{
	const struct uniphier_initdata *initdata;
	int ret;

	assert(soc < ARRAY_SIZE(uniphier_initdata));
	initdata = &uniphier_initdata[soc];

	ret = initdata->memconf_init(dram);
	if (ret) {
		ERROR("failed to init MEMCONF (%d)\n", ret);
		return ret;
	}

	initdata->clk_enable();

	ret = initdata->umc_init(dram);
	if (ret) {
		ERROR("failed to init DRAM (%d)\n", ret);
		return ret;
	}

	return 0;
}

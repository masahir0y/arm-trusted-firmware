
#include <debug.h>
#include <mmio.h>
#include <plat_uniphier.h>
#include <utils.h>

struct uniphier_initdata {
	unsigned int soc_id;
	int (*memconf_init)(const struct uniphier_dram_data *dram);
	void (*clk_enable)(void);
	int (*umc_init)(const struct uniphier_dram_data *dram);
};

static const struct uniphier_initdata uniphier_initdata[] = {
	{
		.soc_id = UNIPHIER_LD11_ID,
		.memconf_init = uniphier_memconf_2ch_init,
		.clk_enable = uniphier_ld11_clk_enable_dram,
		.umc_init = uniphier_ld11_umc_init,
	},
	{
		.soc_id = UNIPHIER_LD20_ID,
		.memconf_init = uniphier_memconf_2ch_init,
		.clk_enable = uniphier_ld20_clk_enable_dram,
		.umc_init = uniphier_ld20_umc_init,
	},
};

static const struct uniphier_initdata *uniphier_get_initdata(unsigned int id)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(uniphier_initdata); i++) {
		if (uniphier_initdata[i].soc_id == id)
			return &uniphier_initdata[i];
	}

	return NULL;
}

int uniphier_soc_setup(unsigned int soc_id,
		       const struct uniphier_dram_data *dram)
{
	const struct uniphier_initdata *initdata;
	int ret;

	initdata = uniphier_get_initdata(soc_id);
	if (!initdata) {
		ERROR("Unsupported SoC ID %d\n", soc_id);
		return -EINVAL;
	}

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

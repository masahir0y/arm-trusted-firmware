#ifndef __DRAM_SETUP_H__
#define __DRAM_SETUP_H__

struct uniphier_dram_data;

struct uniphier_dram_setup_info {
	int (*dram_init)(const struct uniphier_dram_data *dram);
	int (*memconf_init)(const struct uniphier_dram_data *dram);
	uint32_t rst_deassert_bits;
	uint32_t clk_enable_bits;
};

extern struct uniphier_dram_setup_info uniphier_dram_setup_info;

#endif /* __DRAM_SETUP_H__ */

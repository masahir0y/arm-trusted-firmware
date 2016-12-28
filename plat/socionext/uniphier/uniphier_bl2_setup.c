
#include <bl_common.h>
#include <debug.h>
#include <desc_image_load.h>
#include <plat_uniphier.h>
#include <platform_def.h>
#include <xlat_tables.h>

static struct uniphier_board_data uniphier_board_data;

/* Data structure which holds the extents of the trusted SRAM for BL2 */
static meminfo_t uniphier_bl2_tzram_layout __aligned(CACHE_WRITEBACK_GRANULE);

void bl2_early_platform_setup(meminfo_t *mem_layout)
{
	uniphier_bl2_tzram_layout = *mem_layout;

	uniphier_get_board_data(&uniphier_board_data);

	uniphier_console_setup(uniphier_board_data.soc_id,
			       &uniphier_board_data.boot_console);

	/* Initialise the IO layer and register platform IO devices */
	uniphier_io_setup(uniphier_board_data.soc_id);
}

static const struct mmap_region uniphier_bl2_mmap[] = {
	/* for loading BL33 */
	MAP_REGION_FLAT(NS_DRAM_BASE, NS_DRAM_SIZE, MT_MEMORY | MT_RW | MT_NS),
	/* for ROM-API */
	MAP_REGION_FLAT(0x00000000, 0x10000000, MT_CODE | MT_SECURE),
	{ .size = 0 },
};

void bl2_plat_arch_setup(void)
{
	uniphier_mmap_setup(SEC_DRAM_BASE, SEC_DRAM_SIZE, uniphier_bl2_mmap);
	enable_mmu_el1(0);
}

void bl2_platform_setup(void)
{
}

void plat_flush_next_bl_params(void)
{
	flush_bl_params_desc();
}

bl_load_info_t *plat_get_bl_image_load_info(void)
{
	return get_bl_load_info_from_mem_params_desc();
}

bl_params_t *plat_get_next_bl_params(void)
{
	bl_params_t *params;

	params = get_next_bl_params_from_mem_params_desc();

	params->head->ep_info->args.arg1 = (u_register_t)&uniphier_board_data;

	return params;
}

int bl2_plat_handle_post_image_load(unsigned int image_id)
{
	return 0;
}

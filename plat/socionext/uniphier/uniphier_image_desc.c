#include <arch.h>
#include <desc_image_load.h>
#include <platform_def.h>

static bl_mem_params_node_t uniphier_image_descs[] = {
	{
		.image_id = BL31_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
				      VERSION_2, image_info_t,
				      IMAGE_ATTRIB_PLAT_SETUP),
		.image_info.image_base = BL31_BASE,
		.image_info.image_max_size = BL31_LIMIT - BL31_BASE,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
				      VERSION_2, entry_point_info_t,
				      SECURE | EXECUTABLE | EP_FIRST_EXE),
		.ep_info.pc = BL31_BASE,
		.ep_info.spsr = SPSR_64(MODE_EL3, MODE_SP_ELX,
					DISABLE_ALL_EXCEPTIONS),

		.next_handoff_image_id = BL33_IMAGE_ID,
	},

	{
		.image_id = BL33_IMAGE_ID,

		SET_STATIC_PARAM_HEAD(image_info, PARAM_EP,
				      VERSION_2, image_info_t, 0),
		.image_info.image_base = BL33_BASE,
		.image_info.image_max_size = BL33_MAX_SIZE,

		SET_STATIC_PARAM_HEAD(ep_info, PARAM_EP,
				      VERSION_2, entry_point_info_t,
				      NON_SECURE | EXECUTABLE),
		.ep_info.pc = BL33_BASE,
		.ep_info.spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
					DISABLE_ALL_EXCEPTIONS),

		.next_handoff_image_id = INVALID_IMAGE_ID,
	},
};
REGISTER_BL_IMAGE_DESCS(uniphier_image_descs)

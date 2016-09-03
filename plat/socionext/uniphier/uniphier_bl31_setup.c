/*
 * Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <arch.h>
#include <assert.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <mmio.h>
#include <plat_uniphier.h>
#include <platform_def.h>
#include <xlat_tables.h>

#define BL31_END (uintptr_t)(&__BL31_END__)
#define BL31_COHERENT_RAM_BASE (uintptr_t)(&__COHERENT_RAM_START__)
#define BL31_COHERENT_RAM_LIMIT (uintptr_t)(&__COHERENT_RAM_END__)

/*
 * Placeholder variables for copying the arguments that have been passed to
 * BL31 from BL2.
 */
static entry_point_info_t bl32_image_ep_info;
static entry_point_info_t bl33_image_ep_info;

entry_point_info_t *bl31_plat_get_next_image_ep_info(uint32_t type)
{
	assert(sec_state_is_valid(type));
	return type == NON_SECURE ? &bl33_image_ep_info : &bl32_image_ep_info;
}

void bl31_early_platform_setup(bl31_params_t *from_bl2,
			       void *plat_params_from_bl2)
{
	/* Initialize the console to provide early debug support */
	console_init(PLAT_UNIPHIER_BOOT_UART_BASE,
		     PLAT_UNIPHIER_BOOT_UART_CLK_IN_HZ,
		     UNIPHIER_CONSOLE_BAUDRATE);

	/* Populate entry point information for BL33 */
	SET_PARAM_HEAD(&bl33_image_ep_info, PARAM_EP, VERSION_1, 0);
	SET_SECURITY_STATE(bl33_image_ep_info.h.attr, NON_SECURE);

	bl33_image_ep_info.pc = from_bl2->bl33_image_info->image_base;
	bl33_image_ep_info.spsr = SPSR_64(MODE_EL1, MODE_SP_ELX,
					  DISABLE_ALL_EXCEPTIONS);

	/*
	 * Initialize Interconnect for this cluster during cold boot.
	 * No need for locks as no other CPU is active.
	 */
	plat_uniphier_cci_init();

	/*
	 * Enable Interconnect coherency for the primary CPU's cluster.
	 * Platform specific PSCI code will enable coherency for other
	 * clusters.
	 */
	plat_uniphier_cci_enable();
}

#define UNIPHIER_SYS_CNTCTL_BASE	0x60E00000

void bl31_platform_setup(void)
{
	/* Initialize the GIC driver, cpu and distributor interfaces */
	plat_uniphier_gic_driver_init();
	plat_uniphier_gic_init();

	/* Enable and initialize the System level generic timer */
	mmio_write_32(UNIPHIER_SYS_CNTCTL_BASE + CNTCR_OFF,
			CNTCR_FCREQ(0) | CNTCR_EN);
}

void bl31_plat_arch_setup(void)
{
	uniphier_setup_page_tables(BL31_BASE,
				   BL31_END - BL31_BASE,
				   BL_CODE_BASE,
				   BL_CODE_LIMIT,
				   BL31_COHERENT_RAM_BASE,
				   BL31_COHERENT_RAM_LIMIT);
	enable_mmu_el3(0);
}

/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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

#include <gicv3.h>
#include <platform.h>
#include <platform_def.h>

#include "uniphier.h"

#define UNIPHIER_GICD_BASE			0x5fe00000
#if defined(CONFIG_UNIPHIER_LD11)
#define UNIPHIER_GICR_BASE			0x5fe40000
#elif defined(CONFIG_UNIPHIER_LD20) || defined(CONFIG_UNIPHIER_PXS3)
#define UNIPHIER_GICR_BASE			0x5fe80000
#endif

static uintptr_t rdistif_base_addrs[PLATFORM_CORE_COUNT];

static const unsigned int g0_interrupt_array[] = {
	8,	/* SGI0 */
	14,	/* SGI6 */
};

static const unsigned int g1s_interrupt_array[] = {
	29,	/* Timer */
	9,	/* SGI1 */
	10,	/* SGI2 */
	11,	/* SGI3 */
	12,	/* SGI4 */
	13,	/* SGI5 */
	15,	/* SGI7 */
};

static unsigned int uniphier_mpidr_to_core_pos(u_register_t mpidr)
{
	return plat_core_pos_by_mpidr(mpidr);
}

static const struct gicv3_driver_data uniphier_gic_driver_data = {
	.gicd_base = UNIPHIER_GICD_BASE,
	.gicr_base = UNIPHIER_GICR_BASE,
	.g0_interrupt_num = ARRAY_SIZE(g0_interrupt_array),
	.g1s_interrupt_num = ARRAY_SIZE(g1s_interrupt_array),
	.g0_interrupt_array = g0_interrupt_array,
	.g1s_interrupt_array = g1s_interrupt_array,
	.rdistif_num = PLATFORM_CORE_COUNT,
	.rdistif_base_addrs = rdistif_base_addrs,
	.mpidr_to_core_pos = uniphier_mpidr_to_core_pos,
};

void uniphier_gic_driver_init(void)
{
	gicv3_driver_init(&uniphier_gic_driver_data);
}

void uniphier_gic_init(void)
{
	gicv3_distif_init();
	gicv3_rdistif_init(plat_my_core_pos());
	gicv3_cpuif_enable(plat_my_core_pos());
}

void uniphier_gic_cpuif_enable(void)
{
	gicv3_cpuif_enable(plat_my_core_pos());
}

void uniphier_gic_cpuif_disable(void)
{
	gicv3_cpuif_disable(plat_my_core_pos());
}

void uniphier_gic_pcpu_init(void)
{
	gicv3_rdistif_init(plat_my_core_pos());
}

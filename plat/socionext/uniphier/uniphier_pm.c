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

#include <arch_helpers.h>
#include <debug.h>
#include <mmio.h>
#include <psci.h>

#include "uniphier.h"

#define UNIPHIER_ROM_RSV0	0x59801200

#define UNIPHIER_SLFRSTSEL	0x61843010
#define   UNIPHIER_SLFRSTSEL_MASK	(0x3 << 0)
#define UNIPHIER_SLFRSTCTL	0x61843014
#define   UNIPHIER_SLFRSTCTL_RST	(1 << 0)

uintptr_t uniphier_sec_entrypoint;

void uniphier_warmboot_entrypoint(void);
u_register_t uniphier_holding_pen_release;

/*******************************************************************************
 * Platform handler called when a power domain is about to be turned on. The
 * mpidr determines the CPU to be turned on.
 ******************************************************************************/
int uniphier_pwr_domain_on(u_register_t mpidr)
{
	uniphier_holding_pen_release = mpidr;
	flush_dcache_range((uint64_t)&uniphier_holding_pen_release,
			   sizeof(uniphier_holding_pen_release));

	mmio_write_64(UNIPHIER_ROM_RSV0, (uint64_t)&uniphier_warmboot_entrypoint);
	sev();

	return PSCI_E_SUCCESS;
}

/*******************************************************************************
 * Platform handler called when a power domain has just been powered on after
 * being turned off earlier. The target_state encodes the low power state that
 * each level has woken up from.
 ******************************************************************************/
void uniphier_pwr_domain_on_finish(const psci_power_state_t *target_state)
{

	uniphier_gic_pcpu_init();
	uniphier_gic_cpuif_enable();

	uniphier_cci_enable();
}

static void __dead2 uniphier_system_reset(void)
{
	mmio_clrbits_32(UNIPHIER_SLFRSTSEL, UNIPHIER_SLFRSTSEL_MASK);

	/* Write the Reset Control Register */
	mmio_setbits_32(UNIPHIER_SLFRSTCTL, UNIPHIER_SLFRSTCTL_RST);
	wfi();
	ERROR("UniPhier System Reset: operation not handled.\n");
	panic();
}

static const plat_psci_ops_t uniphier_psci_pm_ops = {
	.pwr_domain_on = uniphier_pwr_domain_on,
	.pwr_domain_on_finish = uniphier_pwr_domain_on_finish,
	.system_reset = uniphier_system_reset,
};

int plat_setup_psci_ops(uintptr_t sec_entrypoint,
			const struct plat_psci_ops **psci_ops)
{
	uniphier_sec_entrypoint = sec_entrypoint;
	flush_dcache_range((uint64_t)&uniphier_sec_entrypoint,
			   sizeof(uniphier_sec_entrypoint));

	*psci_ops = &uniphier_psci_pm_ops;

	return 0;
}

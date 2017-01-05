/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
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
#include <cci.h>
#include <plat_uniphier.h>
#include <utils.h>

#define UNIPHIER_CCI500_BASE	0x5FD00000

static const int cci_map[] = {
	0,
	1,
};

int uniphier_cci_init(void) {




}


/******************************************************************************
 * Helper function to initialize ARM CCI driver.
 *****************************************************************************/
void plat_uniphier_cci_init(void)
{
	switch (uniphier_get_soc_id()) {
	case UNIPHIER_LD20_ID:
		cci_init(CCI500_BASE, cci_map, ARRAY_SIZE(cci_map));
		break;
	default:
		/* no CCI */
		break;
	}
}

/******************************************************************************
 * Helper function to place current master into coherency
 *****************************************************************************/
void plat_uniphier_cci_enable(void)
{
	switch (uniphier_get_soc_id()) {
	case UNIPHIER_LD20_ID:
		cci_enable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(read_mpidr_el1()));
		break;
	default:
		/* no CCI */
		break;
	}
}

/******************************************************************************
 * Helper function to remove current master from coherency
 *****************************************************************************/
void plat_uniphier_cci_disable(void)
{
	switch (uniphier_get_soc_id()) {
	case UNIPHIER_LD20_ID:
		cci_disable_snoop_dvm_reqs(MPIDR_AFFLVL1_VAL(read_mpidr_el1()));
		break;
	default:
		/* no CCI */
		break;
	}
}

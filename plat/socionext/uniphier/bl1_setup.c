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

#include <arch_helpers.h>
#include <bl_common.h>
#include <console.h>
#include <debug.h>
#include <platform.h>
#include <platform_def.h>

#include "uniphier.h"

static struct uniphier_board_data uniphier_board_data;
static struct uniphier_dram_data uniphier_dram_data;

void bl1_early_platform_setup(void)
{
	uniphier_get_board_data(&uniphier_board_data);

	uniphier_console_setup(&uniphier_board_data.boot_console);
}

void bl1_plat_arch_setup(void)
{
	/* The MMU has been enabled in the boot ROM */
}

void bl1_platform_setup(void)
{
	int ret;

	uniphier_get_dram_data(&uniphier_dram_data);
	uniphier_show_board_data(&uniphier_board_data);

	ret = uniphier_show_dram_data(&uniphier_dram_data);
	if (ret)
		plat_error_handler(ret);

	ret = uniphier_dram_setup(&uniphier_dram_data);
	if (ret) {
		ERROR("failed to setup SoC\n");
		plat_error_handler(ret);
	}

	ret = uniphier_io_setup();
	if (ret) {
		ERROR("failed to setup io devices\n");
		plat_error_handler(ret);
	}
}

static meminfo_t uniphier_tzram_layout = {
	.total_base = SEC_DRAM_BASE,
	.total_size = SEC_DRAM_SIZE,
};

meminfo_t *bl1_plat_sec_mem_layout(void)
{
	return &uniphier_tzram_layout;
}

void bl1_init_bl2_mem_layout(const meminfo_t *bl1_mem_layout,
			     meminfo_t *bl2_mem_layout)
{
	*bl2_mem_layout = *bl1_mem_layout;

	flush_dcache_range((uint64_t)bl2_mem_layout, sizeof(*bl2_mem_layout));
}

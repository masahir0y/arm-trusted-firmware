/*
 * Copyright (c) 2014-2016, ARM Limited and Contributors. All rights reserved.
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

#ifndef __PLATFORM_DEF_H__
#define __PLATFORM_DEF_H__

#include <common_def.h>
#include <tbbr/tbbr_img_def.h>

#define PLATFORM_STACK_SIZE 0x400

#define CACHE_WRITEBACK_SHIFT   6
#define CACHE_WRITEBACK_GRANULE (1 << CACHE_WRITEBACK_SHIFT)

/* topology */
#define UNIPHIER_MAX_CPUS_PER_CLUSTER	2
#define UNIPHIER_CLUSTER_COUNT		2

#define PLATFORM_CORE_COUNT		\
	((UNIPHIER_MAX_CPUS_PER_CLUSTER) * (UNIPHIER_CLUSTER_COUNT))

#define PLAT_MAX_PWR_LVL		1

#define PLAT_MAX_OFF_STATE   2    /* tmp */
#define PLAT_MAX_RET_STATE   1    /* tmp */

#define SEC_SRAM_BASE		0x30000000
#define SEC_SRAM_SIZE		0x00010000
#define SEC_DRAM_BASE		0x80000000
#define SEC_DRAM_SIZE		0x01000000
#define NS_DRAM_BASE		0x84000000
#define NS_DRAM_SIZE		0x01000000

#ifndef BL1_PREPAD
#define BL1_PREPAD		0
#endif

#define BL1_RO_BASE		((SEC_SRAM_BASE) + (BL1_PREPAD))
#define BL1_RW_LIMIT		((SEC_SRAM_BASE) + (SEC_SRAM_SIZE))
#define BL1_RW_BASE		((BL1_RW_LIMIT) - 0x2000)
#define BL1_RO_LIMIT		((BL1_RW_BASE) - 0x200)

/* BL2 at the tail of secure DRAM */
#define BL2_LIMIT		((SEC_DRAM_BASE) + (SEC_DRAM_SIZE))
#define BL2_BASE		((BL2_LIMIT) - 0x40000)

/* BL31 at the beginning of the secure DRAM */
#define BL31_BASE		(SEC_DRAM_BASE)
#define BL31_PROGBITS_LIMIT	((BL31_BASE) + 0x10000)
#define BL31_LIMIT		((BL31_BASE) + 0x80000)

#define BL33_BASE		(NS_DRAM_BASE)
#define BL33_MAX_SIZE		0x00100000

#define PLAT_PHY_ADDR_SPACE_SIZE			(1ull << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE			(1ull << 32)

/* GICv3 */
#define PLAT_UNIPHIER_GICD_BASE			0x5fe00000
#if defined(CONFIG_UNIPHIER_LD11)
#define PLAT_UNIPHIER_GICR_BASE			0x5fe40000
#elif defined(CONFIG_UNIPHIER_LD20)
#define PLAT_UNIPHIER_GICR_BASE			0x5fe80000
#endif

/* Console */
#define PLAT_UNIPHIER_BOOT_UART_BASE		0x54006800
#define PLAT_UNIPHIER_BOOT_UART_CLK_IN_HZ	58820000
#define UNIPHIER_CONSOLE_BAUDRATE		115200

#define PLAT_UNIPHIER_CRASH_UART_BASE		(PLAT_UNIPHIER_BOOT_UART_BASE)
#define PLAT_UNIPHIER_CRASH_UART_CLK_IN_HZ	(PLAT_UNIPHIER_BOOT_UART_CLK_IN_HZ)

#define MAX_XLAT_TABLES		20
#define MAX_MMAP_REGIONS	25

#define MAX_IO_HANDLES		4
#define MAX_IO_DEVICES		4
#define MAX_IO_BLOCK_DEVICES	1

#endif /* __PLATFORM_DEF_H__ */

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

#define BL31_BASE		0x80000000
#define BL31_PROGBITS_MAX_SIZE	0x00010000
#define BL31_PROGBITS_LIMIT	((BL31_BASE) + (BL31_PROGBITS_MAX_SIZE))
#define BL31_MAX_SIZE		0x00080000
#define BL31_LIMIT		((BL31_BASE) + (BL31_MAX_SIZE))

#define ADDR_SPACE_SIZE			(1ull << 32)

/* GICv3 */
#define PLAT_UNIPHIER_GICD_BASE			0x5fe00000
#if defined(UNIPHIER_LD11)
#define PLAT_UNIPHIER_GICR_BASE			0x5fe40000
#elif defined(UNIPHIER_LD20)
#define PLAT_UNIPHIER_GICR_BASE			0x5fe80000
#endif

#define ARM_IRQ_SEC_PHY_TIMER		29

#define ARM_IRQ_SEC_SGI_0		8
#define ARM_IRQ_SEC_SGI_1		9
#define ARM_IRQ_SEC_SGI_2		10
#define ARM_IRQ_SEC_SGI_3		11
#define ARM_IRQ_SEC_SGI_4		12
#define ARM_IRQ_SEC_SGI_5		13
#define ARM_IRQ_SEC_SGI_6		14
#define ARM_IRQ_SEC_SGI_7		15

/*
 * Define a list of Group 1 Secure and Group 0 interrupts as per GICv3
 * terminology. On a GICv2 system or mode, the lists will be merged and treated
 * as Group 0 interrupts.
 */
#define PLAT_UNIPHIER_G1S_IRQS		ARM_IRQ_SEC_PHY_TIMER,		\
					ARM_IRQ_SEC_SGI_1,		\
					ARM_IRQ_SEC_SGI_2,		\
					ARM_IRQ_SEC_SGI_3,		\
					ARM_IRQ_SEC_SGI_4,		\
					ARM_IRQ_SEC_SGI_5,		\
					ARM_IRQ_SEC_SGI_7

#define PLAT_UNIPHIER_G0_IRQS		ARM_IRQ_SEC_SGI_0,		\
					ARM_IRQ_SEC_SGI_6

/* Console */
#define PLAT_UNIPHIER_BOOT_UART_BASE		0x54006800
#define PLAT_UNIPHIER_BOOT_UART_CLK_IN_HZ	58820000
#define UNIPHIER_CONSOLE_BAUDRATE		115200

#define PLAT_UNIPHIER_CRASH_UART_BASE		(PLAT_UNIPHIER_BOOT_UART_BASE)
#define PLAT_UNIPHIER_CRASH_UART_CLK_IN_HZ	(PLAT_UNIPHIER_BOOT_UART_CLK_IN_HZ)

#define MAX_XLAT_TABLES		20
#define MAX_MMAP_REGIONS	25

#endif /* __PLATFORM_DEF_H__ */

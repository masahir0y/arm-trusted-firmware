/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
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
#ifndef __PLAT_UNIPHIER_H__
#define __PLAT_UNIPHIER_H__

#include <errno.h>
#include <stdint.h>
#include <types.h>

#define UNIPHIER_LD11_ID			0x31
#define UNIPHIER_LD20_ID			0x32

/* useful macros */
#define SZ_1M				0x00100000
#define SZ_2M				0x00200000
#define SZ_4M				0x00400000
#define SZ_8M				0x00800000
#define SZ_16M				0x01000000
#define SZ_32M				0x02000000
#define SZ_64M				0x04000000
#define SZ_128M				0x08000000
#define SZ_256M				0x10000000
#define SZ_512M				0x20000000

#define SZ_1G				0x40000000
#define SZ_2G				0x80000000

#define abs(x) 				((x) < 0 ? -(x) : (x))

struct uniphier_console_data {
	unsigned int uart_port;
	unsigned int baud_rate;
};

#define UNIPHIER_MAX_BOARD_NAME_LEN	32

struct uniphier_board_data {
	unsigned int soc_id;
	unsigned char board_name[UNIPHIER_MAX_BOARD_NAME_LEN];
	struct uniphier_console_data boot_console;
	struct uniphier_console_data runtime_console;
};

#define UNIPHIER_MAX_DRAM_NR_CH		3

struct uniphier_dram_ch {
	uintptr_t base;
	uintptr_t size;
	unsigned int width;
};

struct uniphier_dram_data {
	unsigned int board_type;
	unsigned int dram_freq;
	unsigned int dram_nr_ch;
	struct uniphier_dram_ch dram_ch[UNIPHIER_MAX_DRAM_NR_CH];
};

void uniphier_get_board_data(struct uniphier_board_data *bd);
void uniphier_get_dram_data(struct uniphier_dram_data *dram);

void uniphier_show_board_data(const struct uniphier_board_data *bd);
int uniphier_show_dram_data(const struct uniphier_dram_data *dram);

unsigned int uniphier_get_soc_id(void);

int uniphier_console_setup(unsigned int soc_id,
			   const struct uniphier_console_data *con);

int uniphier_soc_setup(unsigned int soc_id,
		       const struct uniphier_dram_data *dram);

void uniphier_pinctrl_set_mux(unsigned int pin, unsigned int mux);

void uniphier_ld11_clk_enable_uart(void);
void uniphier_ld11_clk_enable_dram(void);
void uniphier_ld20_clk_enable_uart(void);
void uniphier_ld20_clk_enable_dram(void);

int uniphier_memconf_2ch_init(const struct uniphier_dram_data *dram);
int uniphier_memconf_3ch_init(const struct uniphier_dram_data *dram);

#ifdef CONFIG_UNIPHIER_LD11
int uniphier_ld11_umc_init(const struct uniphier_dram_data *dram);
#else
static inline int uniphier_ld11_umc_init(const struct uniphier_dram_data *dram)
{
	return -ENOTSUP;
}
#endif

#ifdef CONFIG_UNIPHIER_LD20
int uniphier_ld20_umc_init(const struct uniphier_dram_data *dram);
#else
static inline int uniphier_ld20_umc_init(const struct uniphier_dram_data *dram)
{
	return -ENOTSUP;
}
#endif

#define UNIPHIER_BOOT_DEVICE_EMMC	0
#define UNIPHIER_BOOT_DEVICE_NAND	1
#define UNIPHIER_BOOT_DEVICE_NOR	2
#define UNIPHIER_BOOT_DEVICE_USB	3

int uniphier_get_boot_device(unsigned int soc_id);

int uniphier_rom_emmc_init(unsigned int soc_id, uintptr_t *block_dev_spec);
int uniphier_rom_nand_init(unsigned int soc_id, uintptr_t *block_dev_spec);
int uniphier_rom_usb_init(unsigned int soc_id, uintptr_t *block_dev_spec);

int uniphier_io_setup(unsigned int soc_id);

struct mmap_region;
void uniphier_mmap_setup(uintptr_t total_base, size_t total_size,
			 const struct mmap_region *mmap);

#ifdef CONFIG_UNIPHIER_LD20
void plat_uniphier_cci_init(void);
void plat_uniphier_cci_enable(void);
void plat_uniphier_cci_disable(void);
#else
static inline void plat_uniphier_cci_init(void)
{
}
static inline void plat_uniphier_cci_enable(void)
{
}
static inline void plat_uniphier_cci_disable(void)
{
}
#endif

/* GIC */
void plat_uniphier_gic_driver_init(void);
void plat_uniphier_gic_init(void);
void plat_arm_gic_cpuif_enable(void);
void plat_uniphier_gic_cpuif_disable(void);
void plat_uniphier_gic_pcpu_init(void);

unsigned int plat_uniphier_calc_core_pos(u_register_t mpidr);

#endif /* __PLAT_UNIPHIER_H__ */

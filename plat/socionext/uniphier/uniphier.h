#ifndef __UNIPHIER_H__
#define __UNIPHIER_H__

#include <errno.h>
#include <stdint.h>
#include <types.h>

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

int uniphier_console_setup(const struct uniphier_console_data *con);

int uniphier_dram_setup(const struct uniphier_dram_data *dram);

void uniphier_pinctrl_set_mux(unsigned int pin, unsigned int mux);

void uniphier_ld11_clk_enable_uart(void);
void uniphier_ld11_clk_enable_dram(void);
void uniphier_ld20_clk_enable_uart(void);
void uniphier_ld20_clk_enable_dram(void);
void uniphier_pxs3_clk_enable_uart(void);
void uniphier_pxs3_clk_enable_dram(void);

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

#ifdef CONFIG_UNIPHIER_PXS3
int uniphier_pxs3_umc_init(const struct uniphier_dram_data *dram);
#else
static inline int uniphier_pxs3_umc_init(const struct uniphier_dram_data *dram)
{
	return -ENOTSUP;
}
#endif

#define UNIPHIER_BOOT_DEVICE_EMMC	0
#define UNIPHIER_BOOT_DEVICE_NAND	1
#define UNIPHIER_BOOT_DEVICE_NOR	2
#define UNIPHIER_BOOT_DEVICE_USB	3

int uniphier_get_boot_device(void);

int uniphier_rom_emmc_init(uintptr_t *block_dev_spec);
int uniphier_rom_nand_init(uintptr_t *block_dev_spec);
int uniphier_rom_usb_init(uintptr_t *block_dev_spec);

int uniphier_io_setup(void);

struct mmap_region;
void uniphier_mmap_setup(uintptr_t total_base, size_t total_size,
			 const struct mmap_region *mmap);

#if defined(CONFIG_UNIPHIER_LD20)
void uniphier_cci_init(void);
void uniphier_cci_enable(void);
void uniphier_cci_disable(void);
#else
static inline void uniphier_cci_init(void)
{
}
static inline void uniphier_cci_enable(void)
{
}
static inline void uniphier_cci_disable(void)
{
}
#endif

/* GIC */
void uniphier_gic_driver_init();
void uniphier_gic_init(void);
void uniphier_gic_cpuif_enable(void);
void uniphier_gic_cpuif_disable(void);
void uniphier_gic_pcpu_init(void);

unsigned int plat_uniphier_calc_core_pos(u_register_t mpidr);

#endif /* __UNIPHIER_H__ */

/*
 * Copyright (C) 2016 Socionext Inc.
 *
 * based on commit a7a36122aa072fe1bb06e02b73b3634b7a6c555a of Diag
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <assert.h>
#include <debug.h>
#include <mmio.h>
#include <plat_uniphier.h>

#include "ddruqphy-regs.h"
#include "umc64-regs.h"

#define DIV_ROUND_CLOSEST(x, divisor)	\
	(((x) + (divisor) / 2) / (divisor))

#define DRAM_CH_NR	3

enum dram_freq {
	DRAM_FREQ_1866M,
	DRAM_FREQ_NR,
};

enum dram_size {
	DRAM_SZ_256M,
	DRAM_SZ_512M,
	DRAM_SZ_NR,
};

enum dram_board {		/* board type */
	DRAM_BOARD_LD20_REF,	/* LD20 reference */
	DRAM_BOARD_LD20_GLOBAL,	/* LD20 TV */
	DRAM_BOARD_LD20_C1,	/* LD20 TV C1 */
	DRAM_BOARD_LD21_REF,	/* LD21 reference */
	DRAM_BOARD_LD21_GLOBAL,	/* LD21 TV */
	DRAM_BOARD_NR,
};

/* PHY */
static const int ddrphy_adrctrl[DRAM_BOARD_NR][DRAM_CH_NR] = {
	{268 - 262, 268 - 263, 268 - 378},	/* LD20 reference */
	{268 - 262, 268 - 263, 268 - 378},	/* LD20 TV */
	{268 - 262, 268 - 263, 268 - 378},	/* LD20 TV C1 */
	{268 - 212, 268 - 268, /* No CH2 */},	/* LD21 reference */
	{268 - 212, 268 - 268, /* No CH2 */},	/* LD21 TV */
};

static const int ddrphy_dlltrimclk[DRAM_BOARD_NR][DRAM_CH_NR] = {
	{268, 268, 268},			/* LD20 reference */
	{268, 268, 268},			/* LD20 TV */
	{189, 189, 189},			/* LD20 TV C1 */
	{268, 268 + 252, /* No CH2 */},		/* LD21 reference */
	{268, 268 + 202, /* No CH2 */},		/* LD21 TV */
};

static const int ddrphy_dllrecalib[DRAM_BOARD_NR][DRAM_CH_NR] = {
	{268 - 378, 268 - 263, 268 - 378},	/* LD20 reference */
	{268 - 378, 268 - 263, 268 - 378},	/* LD20 TV */
	{268 - 378, 268 - 263, 268 - 378},	/* LD20 TV C1 */
	{268 - 212, 268 - 536, /* No CH2 */},	/* LD21 reference */
	{268 - 212, 268 - 536, /* No CH2 */},	/* LD21 TV */
};

static const uint32_t ddrphy_phy_pad_ctrl[DRAM_BOARD_NR][DRAM_CH_NR] = {
	{0x50B840B1, 0x50B840B1, 0x50B840B1},	/* LD20 reference */
	{0x50BB40B1, 0x50BB40B1, 0x50BB40B1},	/* LD20 TV */
	{0x50BB40B1, 0x50BB40B1, 0x50BB40B1},	/* LD20 TV C1 */
	{0x50BB40B4, 0x50B840B1, /* No CH2 */},	/* LD21 reference */
	{0x50BB40B4, 0x50B840B1, /* No CH2 */},	/* LD21 TV */
};

static const uint32_t ddrphy_scl_gate_timing[DRAM_CH_NR] = {
	0x00000140, 0x00000180, 0x00000140
};

static const int ddrphy_op_dq_shift_val[DRAM_BOARD_NR][DRAM_CH_NR][32] = {
	{ /* LD20 reference */
		{
			2, 1, 0, 1, 2, 1, 1, 1,
			2, 1, 1, 2, 1, 1, 1, 1,
			1, 2, 1, 1, 1, 2, 1, 1,
			2, 2, 0, 1, 1, 2, 2, 1,
		},
		{
			1, 1, 0, 1, 2, 2, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 0, 0, 1, 1, 0, 0,
			0, 1, 1, 1, 2, 1, 2, 1,
		},
		{
			2, 2, 0, 2, 1, 1, 2, 1,
			1, 1, 0, 1, 1, -1, 1, 1,
			2, 2, 2, 2, 1, 1, 1, 1,
			1, 1, 1, 0, 2, 2, 1, 2,
		},
	},
	{ /* LD20 TV */
		{
			2, 1, 0, 1, 2, 1, 1, 1,
			2, 1, 1, 2, 1, 1, 1, 1,
			1, 2, 1, 1, 1, 2, 1, 1,
			2, 2, 0, 1, 1, 2, 2, 1,
		},
		{
			1, 1, 0, 1, 2, 2, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 0, 0, 1, 1, 0, 0,
			0, 1, 1, 1, 2, 1, 2, 1,
		},
		{
			2, 2, 0, 2, 1, 1, 2, 1,
			1, 1, 0, 1, 1, -1, 1, 1,
			2, 2, 2, 2, 1, 1, 1, 1,
			1, 1, 1, 0, 2, 2, 1, 2,
		},
	},
	{ /* LD20 TV C1 */
		{
			2, 1, 0, 1, 2, 1, 1, 1,
			2, 1, 1, 2, 1, 1, 1, 1,
			1, 2, 1, 1, 1, 2, 1, 1,
			2, 2, 0, 1, 1, 2, 2, 1,
		},
		{
			1, 1, 0, 1, 2, 2, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 0, 0, 1, 1, 0, 0,
			0, 1, 1, 1, 2, 1, 2, 1,
		},
		{
			2, 2, 0, 2, 1, 1, 2, 1,
			1, 1, 0, 1, 1, -1, 1, 1,
			2, 2, 2, 2, 1, 1, 1, 1,
			1, 1, 1, 0, 2, 2, 1, 2,
		},
	},
	{ /* LD21 reference */
		{
			1, 1, 0, 1, 1, 1, 1, 1,
			1, 0, 0, 0, 1, 1, 0, 2,
			1, 1, 0, 0, 1, 1, 1, 1,
			1, 0, 0, 0, 1, 0, 0, 1,
		},
		{	1, 0, 2, 1, 1, 1, 1, 0,
			1, 0, 0, 1, 0, 1, 0, 0,
			1, 0, 1, 0, 1, 1, 1, 0,
			1, 1, 1, 1, 0, 1, 0, 0,
		},
		/* No CH2 */
	},
	{ /* LD21 TV */
		{
			1, 1, 0, 1, 1, 1, 1, 1,
			1, 0, 0, 0, 1, 1, 0, 2,
			1, 1, 0, 0, 1, 1, 1, 1,
			1, 0, 0, 0, 1, 0, 0, 1,
		},
		{	1, 0, 2, 1, 1, 1, 1, 0,
			1, 0, 0, 1, 0, 1, 0, 0,
			1, 0, 1, 0, 1, 1, 1, 0,
			1, 1, 1, 1, 0, 1, 0, 0,
		},
		/* No CH2 */
	},
};

static int ddrphy_ip_dq_shift_val[DRAM_BOARD_NR][DRAM_CH_NR][32] = {
	{ /* LD20 reference */
		{
			3, 3, 3, 2, 3, 2, 0, 2,
			2, 3, 3, 1, 2, 2, 2, 2,
			2, 2, 2, 2, 0, 1, 1, 1,
			2, 2, 2, 2, 3, 0, 2, 2,
		},
		{
			2, 2, 1, 1, -1, 1, 1, 1,
			2, 0, 2, 2, 2, 1, 0, 2,
			2, 1, 2, 1, 0, 1, 1, 1,
			2, 2, 2, 2, 2, 2, 2, 2,
		},
		{
			2, 2, 3, 2, 1, 2, 2, 2,
			2, 3, 4, 2, 3, 4, 3, 3,
			2, 2, 1, 2, 1, 1, 1, 1,
			2, 2, 2, 2, 1, 2, 2, 1,
		},
	},
	{ /* LD20 TV */
		{
			3, 3, 3, 2, 3, 2, 0, 2,
			2, 3, 3, 1, 2, 2, 2, 2,
			2, 2, 2, 2, 0, 1, 1, 1,
			2, 2, 2, 2, 3, 0, 2, 2,
		},
		{
			2, 2, 1, 1, -1, 1, 1, 1,
			2, 0, 2, 2, 2, 1, 0, 2,
			2, 1, 2, 1, 0, 1, 1, 1,
			2, 2, 2, 2, 2, 2, 2, 2,
		},
		{
			2, 2, 3, 2, 1, 2, 2, 2,
			2, 3, 4, 2, 3, 4, 3, 3,
			2, 2, 1, 2, 1, 1, 1, 1,
			2, 2, 2, 2, 1, 2, 2, 1,
		},
	},
	{ /* LD20 TV C1 */
		{
			3, 3, 3, 2, 3, 2, 0, 2,
			2, 3, 3, 1, 2, 2, 2, 2,
			2, 2, 2, 2, 0, 1, 1, 1,
			2, 2, 2, 2, 3, 0, 2, 2,
		},
		{
			2, 2, 1, 1, -1, 1, 1, 1,
			2, 0, 2, 2, 2, 1, 0, 2,
			2, 1, 2, 1, 0, 1, 1, 1,
			2, 2, 2, 2, 2, 2, 2, 2,
		},
		{
			2, 2, 3, 2, 1, 2, 2, 2,
			2, 3, 4, 2, 3, 4, 3, 3,
			2, 2, 1, 2, 1, 1, 1, 1,
			2, 2, 2, 2, 1, 2, 2, 1,
		},
	},
	{ /* LD21 reference */
		{
			2, 2, 2, 2, 1, 2, 2, 2,
			2, 3, 3, 2, 2, 2, 2, 2,
			2, 1, 2, 2, 1, 1, 1, 1,
			2, 2, 2, 3, 1, 2, 2, 2,
		},
		{
			3, 4, 4, 1, 0, 1, 1, 1,
			1, 2, 1, 2, 2, 3, 3, 2,
			1, 0, 2, 1, 1, 0, 1, 0,
			0, 1, 0, 0, 1, 1, 0, 1,
		},
		/* No CH2 */
	},
	{ /* LD21 TV */
		{
			2, 2, 2, 2, 1, 2, 2, 2,
			2, 3, 3, 2, 2, 2, 2, 2,
			2, 1, 2, 2, 1, 1, 1, 1,
			2, 2, 2, 3, 1, 2, 2, 2,
		},
		{
			3, 4, 4, 1, 0, 1, 1, 1,
			1, 2, 1, 2, 2, 3, 3, 2,
			1, 0, 2, 1, 1, 0, 1, 0,
			0, 1, 0, 0, 1, 1, 0, 1,
		},
		/* No CH2 */
	},
};

/* DDR PHY */
static void ddrphy_select_lane(uintptr_t phy_base, unsigned int lane,
			       unsigned int bit)
{
	assert(lane < 1 << PHY_LANE_SEL_LANE_WIDTH);
	assert(bit < 1 << PHY_LANE_SEL_BIT_WIDTH);

	mmio_write_32(phy_base + PHY_LANE_SEL,
		      (bit << PHY_LANE_SEL_BIT_SHIFT) | (lane << PHY_LANE_SEL_LANE_SHIFT));
}

static void ddrphy_init(uintptr_t phy_base, enum dram_board board, int ch)
{
	mmio_write_32(phy_base + PHY_UNIQUIFY_TSMC_IO_1, 0x0C001001);
	while (!(mmio_read_32(phy_base + PHY_UNIQUIFY_TSMC_IO_1) & BIT(1)))
		;
	mmio_write_32(phy_base + PHY_UNIQUIFY_TSMC_IO_1, 0x0C001000);

	mmio_write_32(phy_base + PHY_DLL_INCR_TRIM_3, 0x00000000);
	mmio_write_32(phy_base + PHY_DLL_INCR_TRIM_1, 0x00000000);
	ddrphy_select_lane(phy_base, 0, 0);
	mmio_write_32(phy_base + PHY_DLL_TRIM_1, 0x00000005);
	mmio_write_32(phy_base + PHY_DLL_TRIM_3, 0x0000000a);
	ddrphy_select_lane(phy_base, 6, 0);
	mmio_write_32(phy_base + PHY_DLL_TRIM_1, 0x00000005);
	mmio_write_32(phy_base + PHY_DLL_TRIM_3, 0x0000000a);
	ddrphy_select_lane(phy_base, 12, 0);
	mmio_write_32(phy_base + PHY_DLL_TRIM_1, 0x00000005);
	mmio_write_32(phy_base + PHY_DLL_TRIM_3, 0x0000000a);
	ddrphy_select_lane(phy_base, 18, 0);
	mmio_write_32(phy_base + PHY_DLL_TRIM_1, 0x00000005);
	mmio_write_32(phy_base + PHY_DLL_TRIM_3, 0x0000000a);
	mmio_write_32(phy_base + PHY_SCL_WINDOW_TRIM, 0x00000001);
	mmio_write_32(phy_base + PHY_UNQ_ANALOG_DLL_1, 0x00000000);
	mmio_write_32(phy_base + PHY_PAD_CTRL, ddrphy_phy_pad_ctrl[board][ch]);
	mmio_write_32(phy_base + PHY_VREF_TRAINING, 0x00000070);
	mmio_write_32(phy_base + PHY_SCL_CONFIG_1, 0x01000075);
	mmio_write_32(phy_base + PHY_SCL_CONFIG_2, 0x00000501);
	mmio_write_32(phy_base + PHY_SCL_CONFIG_3, 0x00000000);
	mmio_write_32(phy_base + PHY_DYNAMIC_WRITE_BIT_LVL, 0x000261c0);
	mmio_write_32(phy_base + PHY_SCL_CONFIG_4, 0x00000000);
	mmio_write_32(phy_base + PHY_SCL_GATE_TIMING,
		      ddrphy_scl_gate_timing[ch]);
	mmio_write_32(phy_base + PHY_WRLVL_DYN_ODT, 0x02a000a0);
	mmio_write_32(phy_base + PHY_WRLVL_ON_OFF, 0x00840004);
	mmio_write_32(phy_base + PHY_DLL_ADRCTRL, 0x0000020d);
	ddrphy_select_lane(phy_base, 0, 0);
	mmio_write_32(phy_base + PHY_DLL_TRIM_CLK, 0x0000008d);
	mmio_write_32(phy_base + PHY_DLL_RECALIB, 0xa800100d);
	mmio_write_32(phy_base + PHY_SCL_LATENCY, 0x00005076);
}

static int ddrphy_to_dly_step(uintptr_t phy_base, unsigned int freq,
			      int delay)
{
	int mdl;

	mdl = (mmio_read_32(phy_base + PHY_DLL_ADRCTRL) & PHY_DLL_ADRCTRL_MDL_MASK) >>
						PHY_DLL_ADRCTRL_MDL_SHIFT;

	return DIV_ROUND_CLOSEST((long)freq * delay * mdl, 2 * 1000000L);
}

static void ddrphy_set_delay(uintptr_t phy_base, unsigned int reg,
			     uint32_t mask, uint32_t incr, int dly_step)
{
	uint32_t tmp;

	tmp = mmio_read_32(phy_base + reg);
	tmp &= ~mask;
	tmp |= min(abs(dly_step), mask);

	if (dly_step >= 0)
		tmp |= incr;
	else
		tmp &= ~incr;

	mmio_write_32(phy_base + reg, tmp);
}

static void ddrphy_set_dll_recalib(uintptr_t phy_base, int dly_step)
{
	ddrphy_set_delay(phy_base, PHY_DLL_RECALIB,
			 PHY_DLL_RECALIB_TRIM_MASK, PHY_DLL_RECALIB_INCR,
			 dly_step);
}

static void ddrphy_set_dll_adrctrl(uintptr_t phy_base, int dly_step)
{
	ddrphy_set_delay(phy_base, PHY_DLL_ADRCTRL,
			 PHY_DLL_ADRCTRL_TRIM_MASK, PHY_DLL_ADRCTRL_INCR,
			 dly_step);
}

static void ddrphy_set_dll_trim_clk(uintptr_t phy_base, int dly_step)
{
	ddrphy_select_lane(phy_base, 0, 0);

	ddrphy_set_delay(phy_base, PHY_DLL_TRIM_CLK,
			 PHY_DLL_TRIM_CLK_MASK, PHY_DLL_TRIM_CLK_INCR,
			 dly_step);
}

static void ddrphy_init_tail(uintptr_t phy_base, enum dram_board board,
			     unsigned int freq, int ch)
{
	int step;

	step = ddrphy_to_dly_step(phy_base, freq, ddrphy_adrctrl[board][ch]);
	ddrphy_set_dll_adrctrl(phy_base, step);

	step = ddrphy_to_dly_step(phy_base, freq, ddrphy_dlltrimclk[board][ch]);
	ddrphy_set_dll_trim_clk(phy_base, step);

	step = ddrphy_to_dly_step(phy_base, freq, ddrphy_dllrecalib[board][ch]);
	ddrphy_set_dll_recalib(phy_base, step);
}

static void ddrphy_shift_one_dq(uintptr_t phy_base, unsigned int reg,
				uint32_t mask, uint32_t incr, int shift_val)
{
	uint32_t tmp;
	int val;

	tmp = mmio_read_32(phy_base + reg);

	val = tmp & mask;
	if (!(tmp & incr))
		val = -val;

	val += shift_val;

	tmp &= ~(incr | mask);
	tmp |= min(abs(val), mask);
	if (val >= 0)
		tmp |= incr;

	mmio_write_32(phy_base + reg, tmp);
}

static void ddrphy_shift_dq(uintptr_t phy_base, unsigned int reg,
			    uint32_t mask, uint32_t incr, uint32_t override,
			    const int *shift_val_array)
{
	uint32_t tmp;
	int dx, bit;

	tmp = mmio_read_32(phy_base + reg);
	tmp |= override;
	mmio_write_32(phy_base + reg, tmp);

	for (dx = 0; dx < 4; dx++) {
		for (bit = 0; bit < 8; bit++) {
			ddrphy_select_lane(phy_base,
					   (PHY_BITLVL_DLY_WIDTH + 1) * dx,
					   bit);

			ddrphy_shift_one_dq(phy_base, reg, mask, incr,
					    shift_val_array[dx * 8 + bit]);
		}
	}

	ddrphy_select_lane(phy_base, 0, 0);
}

static int ddrphy_training(uintptr_t phy_base, enum dram_board board,
			   int ch)
{
	mmio_write_32(phy_base + PHY_WRLVL_AUTOINC_TRIM, 0x0000000f);
	mmio_write_32(phy_base + PHY_DLL_TRIM_2, 0x00010000);
	mmio_write_32(phy_base + PHY_SCL_START, 0x50000000);

	while (mmio_read_32(phy_base + PHY_SCL_START) & PHY_SCL_START_GO_DONE)
		;

	mmio_write_32(phy_base + PHY_DISABLE_GATING_FOR_SCL, 0x00000000);
	mmio_write_32(phy_base + PHY_SCL_DATA_0, 0xff00ff00);
	mmio_write_32(phy_base + PHY_SCL_DATA_1, 0xff00ff00);
	mmio_write_32(phy_base + PHY_SCL_START_ADDR, 0xFBF8FFFF);
	mmio_write_32(phy_base + PHY_SCL_START, 0x11000000);

	while (mmio_read_32(phy_base + PHY_SCL_START) & PHY_SCL_START_GO_DONE)
		;

	mmio_write_32(phy_base + PHY_SCL_START_ADDR, 0xFBF0FFFF);
	mmio_write_32(phy_base + PHY_SCL_START, 0x30500000);

	while (mmio_read_32(phy_base + PHY_SCL_START) & PHY_SCL_START_GO_DONE)
		;

	mmio_write_32(phy_base + PHY_DISABLE_GATING_FOR_SCL, 0x00000001);
	mmio_write_32(phy_base + PHY_SCL_MAIN_CLK_DELTA, 0x00000010);
	mmio_write_32(phy_base + PHY_SCL_DATA_0, 0x789b3de0);
	mmio_write_32(phy_base + PHY_SCL_DATA_1, 0xf10e4a56);
	mmio_write_32(phy_base + PHY_SCL_START, 0x11000000);

	while (mmio_read_32(phy_base + PHY_SCL_START) & PHY_SCL_START_GO_DONE)
		;

	mmio_write_32(phy_base + PHY_SCL_START, 0x34000000);

	while (mmio_read_32(phy_base + PHY_SCL_START) & PHY_SCL_START_GO_DONE)
		;

	mmio_write_32(phy_base + PHY_DISABLE_GATING_FOR_SCL, 0x00000003);

	mmio_write_32(phy_base + PHY_DYNAMIC_WRITE_BIT_LVL, 0x000261c0);
	mmio_write_32(phy_base + PHY_DYNAMIC_BIT_LVL, 0x00003270);
	mmio_write_32(phy_base + PHY_DSCL_CNT, 0x011BD0C4);

	/* shift ip_dq trim */
	ddrphy_shift_dq(phy_base,
			PHY_IP_DQ_DQS_BITWISE_TRIM,
			PHY_IP_DQ_DQS_BITWISE_TRIM_MASK,
			PHY_IP_DQ_DQS_BITWISE_TRIM_INC,
			PHY_IP_DQ_DQS_BITWISE_TRIM_OVERRIDE,
			ddrphy_ip_dq_shift_val[board][ch]);

	/* shift op_dq trim */
	ddrphy_shift_dq(phy_base,
			PHY_OP_DQ_DM_DQS_BITWISE_TRIM,
			PHY_OP_DQ_DM_DQS_BITWISE_TRIM_MASK,
			PHY_OP_DQ_DM_DQS_BITWISE_TRIM_INC,
			PHY_OP_DQ_DM_DQS_BITWISE_TRIM_OVERRIDE,
			ddrphy_op_dq_shift_val[board][ch]);

	return 0;
}

/* UMC */
static const uint32_t umc_initctla[DRAM_FREQ_NR] = {0x71016D11};
static const uint32_t umc_initctlb[DRAM_FREQ_NR] = {0x07E390AC};
static const uint32_t umc_initctlc[DRAM_FREQ_NR] = {0x00FF00FF};
static const uint32_t umc_drmmr0[DRAM_FREQ_NR] = {0x00000114};
static const uint32_t umc_drmmr2[DRAM_FREQ_NR] = {0x000002a0};

static const uint32_t umc_memconf0a[DRAM_FREQ_NR][DRAM_SZ_NR] = {
	/*  256MB       512MB */
	{0x00000601, 0x00000801},	/* 1866 MHz */
};

static const uint32_t umc_memconf0b[DRAM_FREQ_NR][DRAM_SZ_NR] = {
	/*  256MB       512MB */
	{0x00000120, 0x00000130},	/* 1866 MHz */
};

static const uint32_t umc_memconfch[DRAM_FREQ_NR][DRAM_SZ_NR] = {
	/*  256MB       512MB */
	{0x00033603, 0x00033803},	/* 1866 MHz */
};

static const uint32_t umc_cmdctla[DRAM_FREQ_NR] = {0x060D0D20};
static const uint32_t umc_cmdctlb[DRAM_FREQ_NR] = {0x2D211C08};
static const uint32_t umc_cmdctlc[DRAM_FREQ_NR] = {0x00150C04};
static const uint32_t umc_cmdctle[DRAM_FREQ_NR][DRAM_SZ_NR] = {
	/*  256MB       512MB */
	{0x0049071D, 0x0078071D},	/* 1866 MHz */
};

static const uint32_t umc_rdatactl[DRAM_FREQ_NR] = {0x00000610};
static const uint32_t umc_wdatactl[DRAM_FREQ_NR] = {0x00000204};
static const uint32_t umc_odtctl[DRAM_FREQ_NR] = {0x02000002};
static const uint32_t umc_dataset[DRAM_FREQ_NR] = {0x04000000};

static const uint32_t umc_flowctla[DRAM_FREQ_NR] = {0x0081E01E};
static const uint32_t umc_directbusctrla[DRAM_CH_NR] = {
	0x00000000, 0x00000001, 0x00000001
};

static void umc_poll_phy_init_complete(uintptr_t dc_base)
{
	/* Wait for PHY Init Complete */
	while (!(mmio_read_32(dc_base + UMC_DFISTCTLC) & BIT(0)))
		;
}

static int umc_dc_init(uintptr_t dc_base, unsigned int freq,
		       unsigned long size, int ch)
{
	enum dram_freq freq_e;
	enum dram_size size_e;

	switch (freq) {
	case 1866:
		freq_e = DRAM_FREQ_1866M;
		break;
	default:
		ERROR("unsupported DRAM frequency %ud MHz\n", freq);
		return -EINVAL;
	}

	switch (size) {
	case 0:
		return 0;
	case SZ_256M:
		size_e = DRAM_SZ_256M;
		break;
	case SZ_512M:
		size_e = DRAM_SZ_512M;
		break;
	default:
		ERROR("unsupported DRAM size 0x%08lx (per 16bit) for ch%d\n",
		       size, ch);
		return -EINVAL;
	}

	mmio_write_32(dc_base + UMC_DFICSOVRRD, 0x00000001);
	mmio_write_32(dc_base + UMC_DFITURNOFF, 0x00000000);

	mmio_write_32(dc_base + UMC_INITCTLA, umc_initctla[freq_e]);
	mmio_write_32(dc_base + UMC_INITCTLB, umc_initctlb[freq_e]);
	mmio_write_32(dc_base + UMC_INITCTLC, umc_initctlc[freq_e]);

	mmio_write_32(dc_base + UMC_DRMMR0, umc_drmmr0[freq_e]);
	mmio_write_32(dc_base + UMC_DRMMR1, 0x00000004);
	mmio_write_32(dc_base + UMC_DRMMR2, umc_drmmr2[freq_e]);
	mmio_write_32(dc_base + UMC_DRMMR3, 0x00000000);

	mmio_write_32(dc_base + UMC_MEMCONF0A, umc_memconf0a[freq_e][size_e]);
	mmio_write_32(dc_base + UMC_MEMCONF0B, umc_memconf0b[freq_e][size_e]);
	mmio_write_32(dc_base + UMC_MEMCONFCH, umc_memconfch[freq_e][size_e]);
	mmio_write_32(dc_base + UMC_MEMMAPSET, 0x00000008);

	mmio_write_32(dc_base + UMC_CMDCTLA, umc_cmdctla[freq_e]);
	mmio_write_32(dc_base + UMC_CMDCTLB, umc_cmdctlb[freq_e]);
	mmio_write_32(dc_base + UMC_CMDCTLC, umc_cmdctlc[freq_e]);
	mmio_write_32(dc_base + UMC_CMDCTLE, umc_cmdctle[freq_e][size_e]);

	mmio_write_32(dc_base + UMC_RDATACTL_D0, umc_rdatactl[freq_e]);
	mmio_write_32(dc_base + UMC_RDATACTL_D1, umc_rdatactl[freq_e]);

	mmio_write_32(dc_base + UMC_WDATACTL_D0, umc_wdatactl[freq_e]);
	mmio_write_32(dc_base + UMC_WDATACTL_D1, umc_wdatactl[freq_e]);
	mmio_write_32(dc_base + UMC_ODTCTL_D0, umc_odtctl[freq_e]);
	mmio_write_32(dc_base + UMC_ODTCTL_D1, umc_odtctl[freq_e]);
	mmio_write_32(dc_base + UMC_DATASET, umc_dataset[freq_e]);

	mmio_write_32(dc_base + UMC_DCCGCTL, 0x00400020);
	mmio_write_32(dc_base + UMC_ACSSETA, 0x00000003);
	mmio_write_32(dc_base + UMC_FLOWCTLG, 0x00000103);
	mmio_write_32(dc_base + UMC_ACSSETB, 0x00010200);

	mmio_write_32(dc_base + UMC_FLOWCTLA, umc_flowctla[freq_e]);
	mmio_write_32(dc_base + UMC_FLOWCTLC, 0x00004444);
	mmio_write_32(dc_base + UMC_DFICUPDCTLA, 0x00000000);

	mmio_write_32(dc_base + UMC_FLOWCTLB, 0x00202000);
	mmio_write_32(dc_base + UMC_BSICMAPSET, 0x00000000);
	mmio_write_32(dc_base + UMC_ERRMASKA, 0x00000000);
	mmio_write_32(dc_base + UMC_ERRMASKB, 0x00000000);

	mmio_write_32(dc_base + UMC_DIRECTBUSCTRLA, umc_directbusctrla[ch]);

	mmio_write_32(dc_base + UMC_INITSET, 0x00000001);
	/* Wait for PHY Init Complete */
	while (mmio_read_32(dc_base + UMC_INITSTAT) & BIT(0))
		;

	mmio_write_32(dc_base + UMC_SPCSETB, 0x2A0A0A00);
	mmio_write_32(dc_base + UMC_DFICSOVRRD, 0x00000000);

	return 0;
}

static int umc_ch_init(uintptr_t umc_ch_base, uintptr_t phy_ch_base,
		       enum dram_board board, unsigned int freq,
		       unsigned long size, int ch)
{
	uintptr_t dc_base = umc_ch_base + 0x00011000;
	uintptr_t phy_base = phy_ch_base;
	int ret;

	/* PHY Update Mode (ON) */
	mmio_write_32(dc_base + UMC_DFIPUPDCTLA, 0x8000003f);

	/* deassert PHY reset signals */
	mmio_write_32(dc_base + UMC_DIOCTLA,
		      UMC_DIOCTLA_CTL_NRST | UMC_DIOCTLA_CFG_NRST);

	ddrphy_init(phy_base, board, ch);

	umc_poll_phy_init_complete(dc_base);

	ddrphy_init_tail(phy_base, board, freq, ch);

	ret = umc_dc_init(dc_base, freq, size, ch);
	if (ret)
		return ret;

	ret = ddrphy_training(phy_base, board, ch);
	if (ret)
		return ret;

	return 0;
}

static void um_init(uintptr_t um_base)
{
	mmio_write_32(um_base + UMC_MBUS0, 0x000000ff);
	mmio_write_32(um_base + UMC_MBUS1, 0x000000ff);
	mmio_write_32(um_base + UMC_MBUS2, 0x000000ff);
	mmio_write_32(um_base + UMC_MBUS3, 0x00000001);
	mmio_write_32(um_base + UMC_MBUS4, 0x00000001);
	mmio_write_32(um_base + UMC_MBUS5, 0x00000001);
	mmio_write_32(um_base + UMC_MBUS6, 0x00000001);
	mmio_write_32(um_base + UMC_MBUS7, 0x00000001);
	mmio_write_32(um_base + UMC_MBUS8, 0x00000001);
	mmio_write_32(um_base + UMC_MBUS9, 0x00000001);
	mmio_write_32(um_base + UMC_MBUS10, 0x00000001);
}

int uniphier_ld20_umc_init(const struct uniphier_dram_data *dram)
{
	uintptr_t um_base = 0x5b600000;
	uintptr_t umc_ch_base = 0x5b800000;
	uintptr_t phy_ch_base = 0x6e200000;
	enum dram_board board;
	int ch, ret;

	switch (dram->board_type) {
	case 0:
		board = DRAM_BOARD_LD20_REF;
		break;
	case 1:
		board = DRAM_BOARD_LD20_GLOBAL;
		break;
	case 2:
		board = DRAM_BOARD_LD20_C1;
		break;
	case 3:
		board = DRAM_BOARD_LD21_REF;
		break;
	case 4:
		board = DRAM_BOARD_LD21_GLOBAL;
		break;
	default:
		ERROR("unsupported board type %d\n", dram->board_type);
		return -EINVAL;
	}

	for (ch = 0; ch < dram->dram_nr_ch; ch++) {
		unsigned long size = dram->dram_ch[ch].size;
		unsigned int width = dram->dram_ch[ch].width;

		ret = umc_ch_init(umc_ch_base, phy_ch_base, board,
				  dram->dram_freq, size / (width / 16), ch);
		if (ret) {
			ERROR("failed to initialize UMC ch%d\n", ch);
			return ret;
		}

		umc_ch_base += 0x00200000;
		phy_ch_base += 0x00004000;
	}

	um_init(um_base);

	return 0;
}

/*
 * Copyright (C) 2016 Socionext Inc.
 */

#include <debug.h>
#include <mmio.h>

#include "ddrphy_regs.h"
#include "dram_setup.h"
#include "umc_regs.h"
#include "uniphier.h"

#define clamp(val, lo, hi) min(max(val, lo), hi)

#define DDR_FREQ		1600

#define DRAM_CH_NR	2
#define RANK_BLOCKS_TR	2

enum dram_freq {
	DRAM_FREQ_1600M,
	DRAM_FREQ_NR,
};

enum dram_size {
	DRAM_SZ_256M,
	DRAM_SZ_512M,
	DRAM_SZ_NR,
};

/* PHY */
const int rof_pos_shift_pre[RANK_BLOCKS_TR][2] = { {0, 0}, {0, 0} };
const int rof_neg_shift_pre[RANK_BLOCKS_TR][2] = { {0, 0}, {0, 0} };
const int rof_pos_shift[RANK_BLOCKS_TR][2] = { {-35, -35}, {-35, -35} };
const int rof_neg_shift[RANK_BLOCKS_TR][2] = { {-17, -17}, {-17, -17} };
const int tof_shift[RANK_BLOCKS_TR][2] = { {-50, -50}, {-50, -50} };

/* Register address */
#define PHY_ZQ0CR1	0x00000184
#define PHY_ZQ1CR1	0x00000194
#define PHY_ZQ2CR1	0x000001A4
#define PHY_DX0GCR	0x000001C0
#define PHY_DX0GTR	0x000001F0
#define PHY_DX1GCR	0x00000200
#define PHY_DX1GTR	0x00000230
#define PHY_DX2GCR	0x00000240
#define PHY_DX2GTR	0x00000270
#define PHY_DX3GCR	0x00000280
#define PHY_DX3GTR	0x000002B0

#define PHY_DXMDLR(dx)		(0x000001EC + 0x40 * (dx))
#define PHY_DXLCDLR0(dx)	(0x000001E0 + 0x40 * (dx))
#define PHY_DXLCDLR1(dx)	(0x000001E4 + 0x40 * (dx))
#define PHY_DXLCDLR2(dx)	(0x000001E8 + 0x40 * (dx))
#define PHY_DXBDLR1(dx)		(0x000001D0 + 0x40 * (dx))
#define PHY_DXBDLR2(dx)		(0x000001D4 + 0x40 * (dx))

/* MASK */
#define PHY_ACBD_MASK		0x00FC0000
#define PHY_CK0BD_MASK		0x0000003F
#define PHY_CK1BD_MASK		0x00000FC0
#define PHY_IPRD_MASK		0x000000FF
#define PHY_WLD_MASK(rank)	(0xFF << (8 * (rank)))
#define PHY_DQSGD_MASK(rank)	(0xFF << (8 * (rank)))
#define PHY_DQSGX_MASK		BIT(6)
#define PHY_DSWBD_MASK		0x3F000000	/* bit[29:24] */
#define PHY_DSDQOE_MASK		0x00000FFF

static void ddrphy_maskwritel(uint32_t data, uint32_t mask, uintptr_t addr)
{
	uint32_t value;

	value = (mmio_read_32(addr) & ~(mask)) | (data & mask);
	mmio_write_32(addr, value);
}

static uint32_t ddrphy_maskreadl(uint32_t mask, uintptr_t addr)
{
	return mmio_read_32(addr) & mask;
}

/* step of 0.5T  for PUB-byte */
static uint8_t ddrphy_get_mdl(int dx, uintptr_t phy_base)
{
	return ddrphy_maskreadl(PHY_IPRD_MASK, phy_base + PHY_DXMDLR(dx));
}

/* Calculating step for PUB-byte */
static int ddrphy_hpstep(int delay, int dx, uintptr_t phy_base)
{
	return delay * ddrphy_get_mdl(dx, phy_base) * DDR_FREQ / 1000000;
}

static void ddrphy_vt_ctrl(uintptr_t phy_base, int enable)
{
	uint32_t tmp;

	tmp = mmio_read_32(phy_base + PHY_PGCR1);

	if (enable)
		tmp &= ~PHY_PGCR1_INHVT;
	else
		tmp |= PHY_PGCR1_INHVT;

	mmio_write_32(phy_base + PHY_PGCR1, tmp);

	if (!enable) {
		while (!(mmio_read_32(phy_base + PHY_PGSR1) & PHY_PGSR1_VTSTOP))
			;
	}
}

static void ddrphy_set_ckoffset_qoffset(int delay_ckoffset0, int delay_ckoffset1,
					int delay_qoffset, int enable,
					uintptr_t phy_base)
{
	uint8_t ck_step0, ck_step1;	/* ckoffset_step for clock */
	uint8_t q_step;	/*  qoffset_step for clock */
	int dx;

	dx = 2; /* use dx2 in sLD11 */

	ck_step0 = ddrphy_hpstep(delay_ckoffset0, dx, phy_base);     /* CK-Offset */
	ck_step1 = ddrphy_hpstep(delay_ckoffset1, dx, phy_base);     /* CK-Offset */
	q_step = ddrphy_hpstep(delay_qoffset, dx, phy_base);     /*  Q-Offset */

	ddrphy_vt_ctrl(phy_base, 0);

	/* Q->[23:18], CK1->[11:6], CK0->bit[5:0] */
	if (enable == 1)
		ddrphy_maskwritel((q_step << 18) + (ck_step1 << 6) + ck_step0,
				  PHY_ACBD_MASK | PHY_CK1BD_MASK | PHY_CK0BD_MASK,
				  phy_base + PHY_ACBDLR);

	ddrphy_vt_ctrl(phy_base, 1);
}

static void ddrphy_set_wl_delay_dx(int dx, int r0_delay, int r1_delay,
				   int enable, uintptr_t phy_base)
{
	int rank;
	int delay_wl[4];
	uint32_t wl_mask  = 0;   /* WriteLeveling's Mask  */
	uint32_t wl_value = 0;   /* WriteLeveling's Value */

	delay_wl[0] = r0_delay & 0xfff;
	delay_wl[1] = r1_delay & 0xfff;
	delay_wl[2] = 0;
	delay_wl[3] = 0;

	ddrphy_vt_ctrl(phy_base, 0);

	for (rank = 0; rank < 4; rank++) {
		wl_mask  |= PHY_WLD_MASK(rank);
		/*  WriteLeveling's delay */
		wl_value |= ddrphy_hpstep(delay_wl[rank], dx, phy_base) << (8 * rank);
	}

	if (enable == 1)
		ddrphy_maskwritel(wl_value, wl_mask, phy_base + PHY_DXLCDLR0(dx));

	ddrphy_vt_ctrl(phy_base, 1);
}

static void ddrphy_set_dqsg_delay_dx(int dx, int r0_delay, int r1_delay,
				     int enable, uintptr_t phy_base)
{
	int rank;
	int delay_dqsg[4];
	uint32_t dqsg_mask  = 0;   /* DQSGating_LCDL_delay's Mask  */
	uint32_t dqsg_value = 0;   /* DQSGating_LCDL_delay's Value */

	delay_dqsg[0] = r0_delay;
	delay_dqsg[1] = r1_delay;
	delay_dqsg[2] = 0;
	delay_dqsg[3] = 0;

	ddrphy_vt_ctrl(phy_base, 0);

	for (rank = 0; rank < 4; rank++)  {
		dqsg_mask  |= PHY_DQSGD_MASK(rank);
		 /* DQSGating's delay */
		dqsg_value |= ddrphy_hpstep(delay_dqsg[rank], dx, phy_base) << (8 * rank);
	}

	if (enable == 1)
		ddrphy_maskwritel(dqsg_value, dqsg_mask, phy_base + PHY_DXLCDLR2(dx));

	ddrphy_vt_ctrl(phy_base, 1);
}

static void ddrphy_set_dswb_delay_dx(int dx, int delay, int enable,
				     uintptr_t phy_base)
{
	uint8_t dswb_step;

	ddrphy_vt_ctrl(phy_base, 0);

	dswb_step = ddrphy_hpstep(delay, dx, phy_base);     /* DQS-BDL's delay */

	if (enable == 1)
		ddrphy_maskwritel(dswb_step << 24, PHY_DSWBD_MASK, phy_base + PHY_DXBDLR1(dx));

	ddrphy_vt_ctrl(phy_base, 1);
}

static void ddrphy_set_oe_delay_dx(int dx, int dqs_delay, int dq_delay,
				   int enable, uintptr_t phy_base)
{
	uint8_t dqs_oe_step, dq_oe_step;
	uint32_t wdata;

	ddrphy_vt_ctrl(phy_base, 0);

	/* OE(DQS,DQ) */
	dqs_oe_step = ddrphy_hpstep(dqs_delay, dx, phy_base);     /* DQS-oe's delay */
	dq_oe_step = ddrphy_hpstep(dq_delay, dx, phy_base);     /* DQ-oe's delay */
	wdata = ((dq_oe_step<<6) + dqs_oe_step) & 0xFFF;

	if (enable == 1)
		ddrphy_maskwritel(wdata, PHY_DSDQOE_MASK, phy_base + PHY_DXBDLR2(dx));

	ddrphy_vt_ctrl(phy_base, 1);
}

static void ddrphy_ext_dqsgt(uintptr_t phy_base)
{
	/* Extend DQSGating_window   min:+1T  max:+1T */
	ddrphy_maskwritel(PHY_DQSGX_MASK, PHY_DQSGX_MASK, phy_base + PHY_DSGCR);
}

static void ddrphy_shift_tof_hws(uintptr_t phy_base, const int shift[][2])
{
	int dx, block, byte;
	uint32_t lcdlr1, wdqd;

	ddrphy_vt_ctrl(phy_base, 0);

	for (block = 0; block < RANK_BLOCKS_TR; block++) {
		for (byte = 0; byte < 2; byte++) {
			dx = block * 2 + byte;
			lcdlr1 = mmio_read_32(phy_base + PHY_DXLCDLR1(dx));
			wdqd = lcdlr1 & 0xff;
			wdqd = clamp(wdqd + ddrphy_hpstep(shift[block][byte], dx, phy_base),
				     0U, 0xffU);
			lcdlr1 = (lcdlr1 & ~0xff) | wdqd;
			mmio_write_32(phy_base + PHY_DXLCDLR1(dx), lcdlr1);
			mmio_read_32(phy_base + PHY_DXLCDLR1(dx)); /* relax */
		}
	}

	ddrphy_vt_ctrl(phy_base, 1);
}

static void ddrphy_shift_rof_hws(uintptr_t phy_base, const int pos_shift[][2],
				 const int neg_shift[][2])
{
	int dx, block, byte;
	uint32_t lcdlr1, rdqsd, rdqnsd;

	ddrphy_vt_ctrl(phy_base, 0);

	for (block = 0; block < RANK_BLOCKS_TR; block++) {
		for (byte = 0; byte < 2; byte++) {
			dx = block * 2 + byte;
			lcdlr1 = mmio_read_32(phy_base + PHY_DXLCDLR1(dx));

			/*  DQS LCDL  RDQNSD->[23:16]  RDQSD->[15:8] */
			rdqsd  = (lcdlr1 >> 8) & 0xff;
			rdqnsd = (lcdlr1 >> 16) & 0xff;
			rdqsd  = clamp(rdqsd + ddrphy_hpstep(pos_shift[block][byte], dx, phy_base),
				       0U, 0xffU);
			rdqnsd = clamp(rdqnsd + ddrphy_hpstep(neg_shift[block][byte], dx, phy_base),
				       0U, 0xffU);
			lcdlr1 = (lcdlr1 & ~(0xffff << 8)) | (rdqsd << 8) | (rdqnsd << 16);
			mmio_read_32(phy_base + PHY_DXLCDLR1(dx)); /* relax */
		}
	}

	ddrphy_vt_ctrl(phy_base, 1);
}

static void ddrphy_boot_run_hws(uintptr_t phy_base)
{
	/* Hard Training for DIO */
	mmio_write_32(phy_base + PHY_PIR, 0x0000f401);
	while (!(mmio_read_32(phy_base + PHY_PGSR0) & PHY_PGSR0_IDONE))
		;
}

static void ddrphy_training(uintptr_t phy_base)
{
	/* DIO roffset shift before hard training */
	ddrphy_shift_rof_hws(phy_base, rof_pos_shift_pre, rof_neg_shift_pre);

	/* Hard Training for each CH */
	ddrphy_boot_run_hws(phy_base);

	/* DIO toffset shift after training */
	ddrphy_shift_tof_hws(phy_base, tof_shift);

	/* DIO roffset shift after training */
	ddrphy_shift_rof_hws(phy_base, rof_pos_shift, rof_neg_shift);

	/* Extend DQSGating window  min:+1T  max:+1T */
	ddrphy_ext_dqsgt(phy_base);
}

static void ddrphy_init(uintptr_t phy_base, enum dram_freq freq)
{
	mmio_write_32(phy_base + PHY_PIR, 0x40000000);
	mmio_write_32(phy_base + PHY_PGCR1, 0x0300C4F1);
	mmio_write_32(phy_base + PHY_PTR0, 0x0C807D04);
	mmio_write_32(phy_base + PHY_PTR1, 0x27100578);
	mmio_write_32(phy_base + PHY_PTR2, 0x00083DEF);
	mmio_write_32(phy_base + PHY_PTR3, 0x12061A80);
	mmio_write_32(phy_base + PHY_PTR4, 0x08027100);
	mmio_write_32(phy_base + PHY_DTPR0, 0x9D9CBB66);
	mmio_write_32(phy_base + PHY_DTPR1, 0x1a878400);
	mmio_write_32(phy_base + PHY_DTPR2, 0x50025200);
	mmio_write_32(phy_base + PHY_DSGCR, 0xF004641A);
	mmio_write_32(phy_base + PHY_DCR, 0x0000040B);
	mmio_write_32(phy_base + PHY_MR0, 0x00000d71);
	mmio_write_32(phy_base + PHY_MR1, 0x00000006);
	mmio_write_32(phy_base + PHY_MR2, 0x00000098);
	mmio_write_32(phy_base + PHY_MR3, 0x00000000);

	while (!(mmio_read_32(phy_base + PHY_PGSR0) & PHY_PGSR0_IDONE))
		;

	mmio_write_32(phy_base + PHY_ZQ0CR1, 0x00000059);
	mmio_write_32(phy_base + PHY_ZQ1CR1, 0x00000019);
	mmio_write_32(phy_base + PHY_ZQ2CR1, 0x00000019);
	mmio_write_32(phy_base + PHY_PGCR2, 0x30FC6C20);

	ddrphy_set_ckoffset_qoffset(119, 0, 0, 1, phy_base);
	ddrphy_set_wl_delay_dx(0, 220, 220, 1, phy_base);
	ddrphy_set_wl_delay_dx(1, 160, 160, 1, phy_base);
	ddrphy_set_wl_delay_dx(2, 190, 190, 1, phy_base);
	ddrphy_set_wl_delay_dx(3, 150, 150, 1, phy_base);
	ddrphy_set_dqsg_delay_dx(0, 750, 750, 1, phy_base);
	ddrphy_set_dqsg_delay_dx(1, 750, 750, 1, phy_base);
	ddrphy_set_dqsg_delay_dx(2, 750, 750, 1, phy_base);
	ddrphy_set_dqsg_delay_dx(3, 750, 750, 1, phy_base);
	ddrphy_set_dswb_delay_dx(0, 0, 1, phy_base);
	ddrphy_set_dswb_delay_dx(1, 0, 1, phy_base);
	ddrphy_set_dswb_delay_dx(2, 0, 1, phy_base);
	ddrphy_set_dswb_delay_dx(3, 0, 1, phy_base);
	ddrphy_set_oe_delay_dx(0, 0, 0, 1, phy_base);
	ddrphy_set_oe_delay_dx(1, 0, 0, 1, phy_base);
	ddrphy_set_oe_delay_dx(2, 0, 0, 1, phy_base);
	ddrphy_set_oe_delay_dx(3, 0, 0, 1, phy_base);

	mmio_write_32(phy_base + PHY_DX0GCR, 0x44000E81);
	mmio_write_32(phy_base + PHY_DX1GCR, 0x44000E81);
	mmio_write_32(phy_base + PHY_DX2GCR, 0x44000E81);
	mmio_write_32(phy_base + PHY_DX3GCR, 0x44000E81);
	mmio_write_32(phy_base + PHY_DX0GTR, 0x00055002);
	mmio_write_32(phy_base + PHY_DX1GTR, 0x00055002);
	mmio_write_32(phy_base + PHY_DX2GTR, 0x00055010);
	mmio_write_32(phy_base + PHY_DX3GTR, 0x00055010);
	mmio_write_32(phy_base + PHY_DTCR, 0x930035C7);
	mmio_write_32(phy_base + PHY_PIR, 0x00000003);
	mmio_read_32(phy_base + PHY_PIR);
	while (!(mmio_read_32(phy_base + PHY_PGSR0) & PHY_PGSR0_IDONE))
		;

	mmio_write_32(phy_base + PHY_PIR, 0x00000181);
	mmio_read_32(phy_base + PHY_PIR);
	while (!(mmio_read_32(phy_base + PHY_PGSR0) & PHY_PGSR0_IDONE))
		;

	mmio_write_32(phy_base + PHY_DXCCR, 0x44181884);
	mmio_write_32(phy_base + PHY_GPR1, 0x00000001);
}

/* UMC */
static const uint32_t umc_cmdctla[DRAM_FREQ_NR] = {0x060B0B1C};
static const uint32_t umc_cmdctlb[DRAM_FREQ_NR] = {0x27201806};
static const uint32_t umc_cmdctlc[DRAM_FREQ_NR] = {0x00120B04};
static const uint32_t umc_cmdctle[DRAM_FREQ_NR] = {0x00680607};
static const uint32_t umc_cmdctlf[DRAM_FREQ_NR] = {0x02000200};
static const uint32_t umc_cmdctlg[DRAM_FREQ_NR] = {0x08080808};

static const uint32_t umc_rdatactl[DRAM_FREQ_NR] = {0x00000810};
static const uint32_t umc_wdatactl[DRAM_FREQ_NR] = {0x00000004};
static const uint32_t umc_odtctl[DRAM_FREQ_NR]   = {0x02000002};
static const uint32_t umc_acssetb[DRAM_CH_NR] = {0x00000200, 0x00000203};

static const uint32_t umc_memconfch[DRAM_FREQ_NR] = {0x00023605};

static int umc_dc_init(uintptr_t dc_base, enum dram_freq freq,
		       unsigned long size, int ch)
{
	/* Wait for PHY Init Complete */
	mmio_write_32(dc_base + UMC_CMDCTLA, umc_cmdctla[freq]);
	mmio_write_32(dc_base + UMC_CMDCTLB, umc_cmdctlb[freq]);
	mmio_write_32(dc_base + UMC_CMDCTLC, umc_cmdctlc[freq]);
	mmio_write_32(dc_base + UMC_CMDCTLE, umc_cmdctle[freq]);
	mmio_write_32(dc_base + UMC_CMDCTLF, umc_cmdctlf[freq]);
	mmio_write_32(dc_base + UMC_CMDCTLG, umc_cmdctlg[freq]);

	mmio_write_32(dc_base + UMC_RDATACTL_D0, umc_rdatactl[freq]);
	mmio_write_32(dc_base + UMC_RDATACTL_D1, umc_rdatactl[freq]);

	mmio_write_32(dc_base + UMC_WDATACTL_D0, umc_wdatactl[freq]);
	mmio_write_32(dc_base + UMC_WDATACTL_D1, umc_wdatactl[freq]);

	mmio_write_32(dc_base + UMC_ODTCTL_D0, umc_odtctl[freq]);
	mmio_write_32(dc_base + UMC_ODTCTL_D1, umc_odtctl[freq]);

	mmio_write_32(dc_base + UMC_ACSSETA, 0x00000003);
	mmio_write_32(dc_base + UMC_FLOWCTLG, 0x00000103);
	mmio_write_32(dc_base + UMC_ACSSETB, umc_acssetb[ch]);
	mmio_write_32(dc_base + UMC_SPCSETB, 0x02020200);
	mmio_write_32(dc_base + UMC_MEMCONFCH, umc_memconfch[freq]);
	mmio_write_32(dc_base + UMC_ACFETCHCTRL, 0x00000002);

	return 0;
}

static int umc_ch_init(uintptr_t umc_ch_base,
		       enum dram_freq freq, unsigned long size, int ch)
{
	uintptr_t dc_base  = umc_ch_base;

	return umc_dc_init(dc_base, freq, size, ch);
}

static void um_init(uintptr_t um_base)
{
	mmio_write_32(um_base + UMC_SIORST, 0x00000001);
	mmio_write_32(um_base + UMC_VO0RST, 0x00000001);
	mmio_write_32(um_base + UMC_VPERST, 0x00000001);
	mmio_write_32(um_base + UMC_RGLRST, 0x00000001);
	mmio_write_32(um_base + UMC_A2DRST, 0x00000001);
	mmio_write_32(um_base + UMC_DMDRST, 0x00000001);
}

static int dram_init(const struct uniphier_dram_data *dram)
{
	uintptr_t um_base = 0x5B800000;
	uintptr_t umc_ch_base = 0x5BC00000;
	uintptr_t phy_base = 0x5BC01000;
	enum dram_freq freq;
	int ch, ret;

	switch (dram->dram_freq) {
	case 1600:
		freq = DRAM_FREQ_1600M;
		break;
	default:
		ERROR("unsupported DRAM frequency %d MHz\n", dram->dram_freq);
		return -EINVAL;
	}

	mmio_write_32(umc_ch_base + UMC_DIOCTLA, 0x00000101);
	while (!(mmio_read_32(phy_base + PHY_PGSR0) & PHY_PGSR0_IDONE))
		;

	mmio_write_32(umc_ch_base + UMC_DIOCTLA, 0x00000000);
	mmio_write_32(umc_ch_base + UMC_DEBUGC, 0x00000001);
	mmio_write_32(umc_ch_base + UMC_DIOCTLA, 0x00000101);

	mmio_write_32(umc_ch_base + UMC_INITSET, 0x00000100);
	while (mmio_read_32(umc_ch_base + UMC_INITSTAT) & BIT(8))
		;

	mmio_write_32(umc_ch_base + 0x00200000 + UMC_INITSET, 0x00000100);
	while (mmio_read_32(umc_ch_base + 0x00200000 + UMC_INITSTAT) & BIT(8))
		;

	ddrphy_init(phy_base, freq);

	for (ch = 0; ch < dram->dram_nr_ch; ch++) {
		unsigned long size = dram->dram_ch[ch].size;
		unsigned int width = dram->dram_ch[ch].width;

		ret = umc_ch_init(umc_ch_base, freq, size / (width / 16), ch);
		if (ret) {
			ERROR("failed to initialize UMC ch%d\n", ch);
			return ret;
		}

		umc_ch_base += 0x00200000;
	}
	ddrphy_training(phy_base);

	um_init(um_base);

	return 0;
}

struct uniphier_dram_setup_info uniphier_dram_setup_info = {
	.dram_init = dram_init,
	.memconf_init = uniphier_memconf_2ch_init,
	.rst_deassert_bits = 0x00000003,
	.clk_enable_bits = 0x00000003,
};

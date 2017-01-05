#include <mmio.h>
#include <utils.h>
#include <plat_uniphier.h>

#define UNIPHIER_SYSCTRL_BASE		0x61840000
#define UNIPHIER_RSTCTRL4		((UNIPHIER_SYSCTRL_BASE) + 0x200c)
#define UNIPHIER_RSTCTRL7		((UNIPHIER_SYSCTRL_BASE) + 0x2018)
#define UNIPHIER_CLKCTRL4		((UNIPHIER_SYSCTRL_BASE) + 0x210c)
#define UNIPHIER_CLKCTRL7		((UNIPHIER_SYSCTRL_BASE) + 0x2118)

void uniphier_ld11_clk_enable_uart(void)
{
	mmio_setbits_32(UNIPHIER_CLKCTRL4, BIT(7));	/* PERI */
}

void uniphier_ld11_clk_enable_dram(void)
{
	/* UM[1:0] */
	mmio_setbits_32(UNIPHIER_RSTCTRL7, 0x00000003);
	/* UM[1:0] */
	mmio_setbits_32(UNIPHIER_CLKCTRL7, 0x00000003);
}

void uniphier_ld20_clk_enable_uart(void)
{
	mmio_setbits_32(UNIPHIER_CLKCTRL4, BIT(7));	/* PERI */
}

void uniphier_ld20_clk_enable_dram(void)
{
	/* UMSB, UMA[2:0], UM3[2:0] */
	mmio_setbits_32(UNIPHIER_RSTCTRL7, 0x00010707);
	/* UMSB, UM[2:0] */
	mmio_setbits_32(UNIPHIER_CLKCTRL7, 0x00010007);
}

void uniphier_pxs3_clk_enable_uart(void)
{
	mmio_setbits_32(UNIPHIER_CLKCTRL4, BIT(11));	/* PERI */
}

void uniphier_pxs3_clk_enable_dram(void)
{
	/* PHY[2:0], UMSB, UMA[2:0], UM[2:0] */
	mmio_setbits_32(UNIPHIER_RSTCTRL7, 0x07010707);
	/* UMSB, UM[2:0] */
	mmio_setbits_32(UNIPHIER_CLKCTRL7, 0x00010007);
}

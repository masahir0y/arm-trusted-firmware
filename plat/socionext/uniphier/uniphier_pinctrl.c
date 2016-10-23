#include <mmio.h>

#define UNIPHIER_PINCTRL_BASE		0x5f801000
#define UNIPHIER_PINCTRL_PINMUX_BASE	((UNIPHIER_PINCTRL_BASE) + 0x000)
#define UNIPHIER_PINCTRL_IECTRL_BASE	((UNIPHIER_PINCTRL_BASE) + 0xd00)

#define UNIPHIER_PINCTRL_PINMUX_BITS	8

static void __uniphier_pinctrl_update_field(uintptr_t base, int field_width,
					    int pin, int val)
{
	uint32_t mask, tmp;
	uintptr_t reg;
	int shift;

	reg = base + pin * field_width / 32 * 4;
	shift = pin * field_width % 32;
	mask = (1 << field_width) - 1;

	tmp = mmio_read_32(reg);
	tmp &= ~(mask << shift);
	tmp |= (mask & val) << shift;
	mmio_write_32(reg, tmp);
}

void uniphier_pinctrl_set_mux(unsigned int pin, unsigned int mux)
{
	/* mux */
	__uniphier_pinctrl_update_field(UNIPHIER_PINCTRL_PINMUX_BASE, 8,
					pin, mux);

	/* enable input */
	__uniphier_pinctrl_update_field(UNIPHIER_PINCTRL_PINMUX_BASE, 1,
					pin, 1);
}

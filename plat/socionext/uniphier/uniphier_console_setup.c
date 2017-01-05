#include <console.h>
#include <plat_uniphier.h>
#include <stddef.h>
#include <utils.h>

#define UNIPHIER_NR_UART_PORTS		4

#ifndef UNIPHIER_CRASH_CONSOLE_FIXED
uintptr_t uniphier_crash_console_base;
#endif

struct uniphier_uart_port {
	uintptr_t base;
	unsigned int pin;
	unsigned int mux;
};

struct uniphier_uart_info {
	unsigned int uart_clk;
	struct uniphier_uart_port uart_port[UNIPHIER_NR_UART_PORTS];
	void (*clk_enable)(void);
};

static const struct uniphier_uart_info uniphier_uart_info[] = {
	[UNIPHIER_SOC_LD11] = {
		.uart_clk = 58820000,
		.uart_port = {
			{ .base = 0x54006800, .pin = 54, .mux = 0, },
			{ .base = 0x54006900, .pin = 58, .mux = 1, },
			{ .base = 0x54006a00, .pin = 90, .mux = 1, },
			{ .base = 0x54006b00, .pin = 94, .mux = 1, },
		},
		.clk_enable = uniphier_ld11_clk_enable_uart,
	},
	[UNIPHIER_SOC_LD20] = {
		.uart_clk = 58820000,
		.uart_port = {
			{ .base = 0x54006800, .pin = 54, .mux = 0, },
			{ .base = 0x54006900, .pin = 58, .mux = 1, },
			{ .base = 0x54006a00, .pin = 90, .mux = 1, },
			{ .base = 0x54006b00, .pin = 94, .mux = 1, },
		},
		.clk_enable = uniphier_ld20_clk_enable_uart,
	},
	[UNIPHIER_SOC_PXS3] = {
		.uart_clk = 58820000,
		.uart_port = {
			{ .base = 0x54006800, .pin = 92, .mux = 0, },
			{ .base = 0x54006900, .pin = 94, .mux = 0, },
			{ .base = 0x54006a00, .pin = 96, .mux = 0, },
			{ .base = 0x54006b00, .pin = 98, .mux = 0, },
		},
		.clk_enable = uniphier_pxs3_clk_enable_uart,
	},
};

int uniphier_console_setup(unsigned int soc,
			   const struct uniphier_console_data *con)
{
	const struct uniphier_uart_info *info;
	const struct uniphier_uart_port *port;

	if (con->uart_port >= UNIPHIER_NR_UART_PORTS)
		return -EINVAL;

	assert(soc < ARRAY_SIZE(uniphier_uart_info));
	info = &uniphier_uart_info[soc];
	port = &info->uart_port[con->uart_port];

	uniphier_pinctrl_set_mux(port->pin, port->mux);

	info->clk_enable();

	console_init(port->base, info->uart_clk, con->baud_rate);

#ifndef UNIPHIER_CRASH_CONSOLE_FIXED
	uniphier_crash_console_base = uniphier_crash_console_base;
#endif

	return 0;
}

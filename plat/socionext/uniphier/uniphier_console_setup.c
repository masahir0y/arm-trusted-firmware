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
	unsigned int soc_id;
	unsigned int uart_clk;
	struct uniphier_uart_port uart_port[UNIPHIER_NR_UART_PORTS];
	void (*clk_enable)(void);
};

static const struct uniphier_uart_info uniphier_uart_info[] = {
	{
		.soc_id = UNIPHIER_LD11_ID,
		.uart_clk = 58820000,
		.uart_port = {
			{ .base = 0x54006800, .pin = 54, .mux = 0, },
			{ .base = 0x54006900, .pin = 58, .mux = 1, },
			{ .base = 0x54006a00, .pin = 90, .mux = 1, },
			{ .base = 0x54006b00, .pin = 94, .mux = 1, },
		},
		.clk_enable = uniphier_ld11_clk_enable_uart,
	},
	{
		.soc_id = UNIPHIER_LD20_ID,
		.uart_clk = 58820000,
		.uart_port = {
			{ .base = 0x54006800, .pin = 54, .mux = 0, },
			{ .base = 0x54006900, .pin = 58, .mux = 1, },
			{ .base = 0x54006a00, .pin = 90, .mux = 1, },
			{ .base = 0x54006b00, .pin = 94, .mux = 1, },
		},
		.clk_enable = uniphier_ld20_clk_enable_uart,
	},
};

static const struct uniphier_uart_info *uniphier_get_uart_info(unsigned int id)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(uniphier_uart_info); i++) {
		if (uniphier_uart_info[i].soc_id == id)
			return &uniphier_uart_info[i];
	}

	return NULL;
}

int uniphier_console_setup(unsigned int soc_id,
			   const struct uniphier_console_data *con)
{
	const struct uniphier_uart_info *info;
	const struct uniphier_uart_port *port;

	if (con->uart_port >= UNIPHIER_NR_UART_PORTS)
		return -EINVAL;

	info = uniphier_get_uart_info(soc_id);
	if (!info)
		return -EINVAL;

	port = &info->uart_port[con->uart_port];

	uniphier_pinctrl_set_mux(port->pin, port->mux);

	info->clk_enable();

	console_init(port->base, info->uart_clk, con->baud_rate);

#ifndef UNIPHIER_CRASH_CONSOLE_FIXED
	uniphier_crash_console_base = uniphier_crash_console_base;
#endif

	return 0;
}

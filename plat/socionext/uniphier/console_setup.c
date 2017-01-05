#include <console.h>
#include <mmio.h>

#include "uniphier.h"
#include "uniphier_uart.h"

#define UNIPHIER_CLKCTRL4	0x6184210c

#ifndef UNIPHIER_CRASH_CONSOLE_FIXED
uintptr_t uniphier_crash_console_base;
#endif

int uniphier_console_setup(const struct uniphier_console_data *con)
{
	const struct uniphier_uart_info *info = &uniphier_uart_info;
	const struct uniphier_uart_port *port;

	if (con->uart_port >= info->nr_ports)
		return -EINVAL;

	port = &info->ports[con->uart_port];

	uniphier_pinctrl_set_mux(port->pin, port->mux);

	mmio_setbits_32(UNIPHIER_CLKCTRL4, info->clk_enable_bits);

	console_init(port->base, info->clk_rate, con->baud_rate);

#ifndef UNIPHIER_CRASH_CONSOLE_FIXED
	uniphier_crash_console_base = uniphier_crash_console_base;
#endif

	return 0;
}


#include <utils.h>

#include "uniphier_uart.h"

static const struct uniphier_uart_port uart_ports[] = {
	{ .base = 0x54006800, .pin = 92, .mux = 0, },
	{ .base = 0x54006900, .pin = 94, .mux = 0, },
	{ .base = 0x54006a00, .pin = 96, .mux = 0, },
	{ .base = 0x54006b00, .pin = 98, .mux = 0, },
};

const struct uniphier_uart_info uniphier_uart_info = {
	.ports = uart_ports,
	.nr_ports = ARRAY_SIZE(uart_ports),
	.clk_rate = 58823529,
	.clk_enable_bits = BIT(11),
};

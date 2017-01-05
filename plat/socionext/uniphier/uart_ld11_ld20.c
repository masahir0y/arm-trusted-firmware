
#include <utils.h>

#include "uniphier_uart.h"

static const struct uniphier_uart_port uniphier_uart_ports[] = {
	{ .base = 0x54006800, .pin = 54, .mux = 0, },
	{ .base = 0x54006900, .pin = 58, .mux = 1, },
	{ .base = 0x54006a00, .pin = 90, .mux = 1, },
	{ .base = 0x54006b00, .pin = 94, .mux = 1, },
};

const struct uniphier_uart_info uniphier_uart_info = {
	.ports = uniphier_uart_ports,
	.nr_ports = ARRAY_SIZE(uniphier_uart_ports),
	.clk_rate = 58823529,
	.clk_enable_bits = BIT(7),
};

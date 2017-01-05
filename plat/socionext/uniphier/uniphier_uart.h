
#ifndef __UNIPHIER_UART_H__
#define __UNIPHIER_UART_H__

#include <sys/types.h>

struct uniphier_uart_port {
	uintptr_t base;
	unsigned int pin;
	unsigned int mux;
};

struct uniphier_uart_info {
	const struct uniphier_uart_port *ports;
	unsigned int nr_ports;
	unsigned int clk_rate;
	uint32_t clk_enable_bits;
};

extern const struct uniphier_uart_info uniphier_uart_info;

#endif /* __UNIPHIER_UART__ */

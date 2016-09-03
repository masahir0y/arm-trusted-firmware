/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <asm_macros.S>

#define UNIPHIER_UART_RX	0x0	/* In:  Receive buffer */
#define UNIPHIER_UART_TX	0x0	/* Out: Transmit buffer */

#define UNIPHIER_UART_LCR_MCR	0x10	/* Line/Modem Control Register */
#define   UNIPHIER_UART_LCR_WLEN8	0x03	/* Wordlength: 8 bits */
#define UNIPHIER_UART_LSR	0x14	/* Line Status Register */
#define   UNIPHIER_UART_LSR_THRE	0x20 /* Transmit-hold-register empty */
#define   UNIPHIER_UART_LSR_DR		0x01 /* Receiver data ready */
#define UNIPHIER_UART_DLR	0x24	/* Divisor Latch Register */

	/*
	 * UniPhier serial console implemented based on
	 * drivers/console/aarch64/skeleton_console.S
	 */

	.globl	console_core_init
	.globl	console_core_putc
	.globl	console_core_getc

	/* -----------------------------------------------
	 * int console_core_init(uintptr_t base_addr,
	 * unsigned int uart_clk, unsigned int baud_rate)
	 * Function to initialize the console without a
	 * C Runtime to print debug information. This
	 * function will be accessed by console_init and
	 * crash reporting.
	 * In: x0 - console base address
	 *     w1 - Uart clock in Hz
	 *     w2 - Baud rate
	 * Out: return 1 on success else 0 on error
	 * Clobber list : x1, x2
	 * -----------------------------------------------
	 */
func console_core_init
	/* Check the input base address */
	cbz	x0, core_init_fail
	/* Check baud rate and uart clock for sanity */
	cbz	w1, core_init_fail
	cbz	w2, core_init_fail

	/* Divisor = Uart_clock / (16 * Baudrate) */
	udiv	w2, w1, w2
	lsr	w2, w2, #4
	str	w2, [x0, #UNIPHIER_UART_DLR]

	mov	w2, #(UNIPHIER_UART_LCR_WLEN8 << 8)
	str	w2, [x0, #UNIPHIER_UART_LCR_MCR]

	mov	w0, #1
	ret
core_init_fail:
	mov	w0, wzr
	ret
endfunc console_core_init

	/* --------------------------------------------------------
	 * int console_core_putc(int c, uintptr_t base_addr)
	 * Function to output a character over the console. It
	 * returns the character printed on success or -1 on error.
	 * In : w0 - character to be printed
	 *      x1 - console base address
	 * Out : return -1 on error else return character.
	 * Clobber list : x2
	 * --------------------------------------------------------
	 */
func console_core_putc
	/* Check the input parameter */
	cbz	x1, putc_error

	/* Wait until the transmit FIFO gets empty */
1:	ldr	w2, [x1, #UNIPHIER_UART_LSR]
	and	w2, w2, #UNIPHIER_UART_LSR_THRE
	cbz	w2, 1b

	str	w0, [x1, #UNIPHIER_UART_TX]

	cmp	w0, #'\n'
	b.ne	2f
	mov	w0, #'\r'	/* Append '\r' to '\n' */
	b	1b
2:
	ret
putc_error:
	mov	w0, #-1
	ret
endfunc console_core_putc

	/* ---------------------------------------------
	 * int console_core_getc(uintptr_t base_addr)
	 * Function to get a character from the console.
	 * It returns the character grabbed on success
	 * or -1 on error.
	 * In : x0 - console base address
	 * Clobber list : x0, x1
	 * ---------------------------------------------
	 */
func console_core_getc
	cbz	x0, getc_error

	/* Wait while the receive FIFO is empty */
1:	ldr	w1, [x0, #UNIPHIER_UART_LSR]
	and	w1, w1, #UNIPHIER_UART_LSR_DR
	cbz	w1, 1b

	ldr	w0, [x0, #UNIPHIER_UART_RX]

	ret
getc_error:
	mov	w0, #-1
	ret
endfunc console_core_getc

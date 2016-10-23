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

#define UNIPHIER_UART_RX	0x00	/* In:  Receive buffer */
#define UNIPHIER_UART_TX	0x00	/* Out: Transmit buffer */

#define UNIPHIER_UART_FCR	0x0c	/* Char/FIFO Control Register */
#define   UNIPHIER_UART_FCR_ENABLE_FIFO	0x01	/* Enable the FIFO */

#define UNIPHIER_UART_LCR_MCR	0x10	/* Line/Modem Control Register */
#define   UNIPHIER_UART_LCR_WLEN8	0x03	/* Wordlength: 8 bits */
#define UNIPHIER_UART_LSR	0x14	/* Line Status Register */
#define   UNIPHIER_UART_LSR_TEMT_BIT	6	/* Transmitter empty */
#define   UNIPHIER_UART_LSR_THRE_BIT	5	/* Transmit-hold-register empty */
#define   UNIPHIER_UART_LSR_DR_BIT	0	/* Receiver data ready */
#define UNIPHIER_UART_DLR	0x24	/* Divisor Latch Register */

	.globl	console_core_init
	.globl	console_core_putc
	.globl	console_core_getc

/*
 * In: x0 - console base address
 *     w1 - uart clock in Hz
 *     w2 - baud rate
 * Out: return 1 (on success)
 * Clobber list : x1, x2
 */
func console_core_init
	cbz	x0, 1f
	cbz	w1, 1f
	cbz	w2, 1f

	/* divisor = uart_clock / (16 * baud_rate) */
	udiv	w2, w1, w2
	lsr	w2, w2, #4

	/* Make sure the transmitter is empty before the divisor set/change */
0:	ldr	w1, [x0, #UNIPHIER_UART_LSR]
	tbz	w1, #UNIPHIER_UART_LSR_TEMT_BIT, 0b

	str	w2, [x0, #UNIPHIER_UART_DLR]

	mov	w2, #UNIPHIER_UART_FCR_ENABLE_FIFO
	str	w2, [x0, #UNIPHIER_UART_FCR]

	mov	w2, #(UNIPHIER_UART_LCR_WLEN8 << 8)
	str	w2, [x0, #UNIPHIER_UART_LCR_MCR]

	mov	w0, #1
	ret
1:	mov	w0, #1
	ret
endfunc console_core_init

/*
 * In : w0 - character to be printed
 *      x1 - console base address
 * Out : return the character written, or -1 on error
 * Clobber list : x1, x2
 */
func console_core_putc
	/* Error out if the console is not initialized */
	cbz	x1, 2f

	/* Wait until the transmitter FIFO gets empty */
0:	ldr	w2, [x1, #UNIPHIER_UART_LSR]
	tbz	w2, #UNIPHIER_UART_LSR_THRE_BIT, 0b

	mov	w2, w0

1:	str	w2, [x1, #UNIPHIER_UART_TX]

	cmp	w2, #'\n'
	b.ne	3f
	mov	w2, #'\r'	/* Append '\r' to '\n' */
	b	1b
2:	mov	w0, #-1
3:	ret
endfunc console_core_putc

/*
 * In : x0 - console base address
 * Out: return the character read
 * Clobber list : x1
 */
func console_core_getc
	/* Error out if the console is not initialized */
	cbz	x0, 1f

	/* Wait while the receiver FIFO is empty */
0:	ldr	w1, [x0, #UNIPHIER_UART_LSR]
	tbz	w1, #UNIPHIER_UART_LSR_DR_BIT, 0b

	ldr	w0, [x0, #UNIPHIER_UART_RX]

	ret
1:	mov	w0, #-1
	ret
endfunc console_core_getc

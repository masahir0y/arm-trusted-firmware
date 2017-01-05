
#include <debug.h>
#include <mmio.h>
#include <platform_def.h>
#include <string.h>

#include "uniphier.h"

#define UNIPHIER_PARAM_BASE			((SEC_SRAM_BASE) + 0x200)

#define UNIPHIER_PARAM_SIGNATURE		((UNIPHIER_PARAM_BASE) + 0x00)
#define UNIPHIER_PARAM_BOARD_NAME		((UNIPHIER_PARAM_BASE) + 0x10)

#define UNIPHIER_PARAM_BOOT_CONSOLE_PORT	((UNIPHIER_PARAM_BASE) + 0x30)
#define UNIPHIER_PARAM_BOOT_CONSOLE_BAUDRATE	((UNIPHIER_PARAM_BASE) + 0x34)
#define UNIPHIER_PARAM_RUNTIME_CONSOLE_PORT	((UNIPHIER_PARAM_BASE) + 0x38)
#define UNIPHIER_PARAM_RUNTIME_CONSOLE_BAUDRATE	((UNIPHIER_PARAM_BASE) + 0x3c)

#define UNIPHIER_PARAM_BOARD_TYPE		((UNIPHIER_PARAM_BASE) + 0x40)
#define UNIPHIER_PARAM_DRAM_FREQ		((UNIPHIER_PARAM_BASE) + 0x44)
#define UNIPHIER_PARAM_DRAM_CH0_BASE		((UNIPHIER_PARAM_BASE) + 0x48)
#define UNIPHIER_PARAM_DRAM_CH0_SIZE		((UNIPHIER_PARAM_BASE) + 0x50)
#define UNIPHIER_PARAM_DRAM_CH0_WIDTH		((UNIPHIER_PARAM_BASE) + 0x58)
#define UNIPHIER_PARAM_DRAM_CH1_BASE		((UNIPHIER_PARAM_BASE) + 0x60)
#define UNIPHIER_PARAM_DRAM_CH1_SIZE		((UNIPHIER_PARAM_BASE) + 0x68)
#define UNIPHIER_PARAM_DRAM_CH1_WIDTH		((UNIPHIER_PARAM_BASE) + 0x70)
#define UNIPHIER_PARAM_DRAM_CH2_BASE		((UNIPHIER_PARAM_BASE) + 0x78)
#define UNIPHIER_PARAM_DRAM_CH2_SIZE		((UNIPHIER_PARAM_BASE) + 0x80)
#define UNIPHIER_PARAM_DRAM_CH2_WIDTH		((UNIPHIER_PARAM_BASE) + 0x88)

void uniphier_get_board_data(struct uniphier_board_data *bd)
{
	memcpy(bd->board_name, (void *)UNIPHIER_PARAM_BOARD_NAME,
	       sizeof(bd->board_name));
	bd->board_name[sizeof(bd->board_name) - 1] = 0;

	bd->boot_console.uart_port =
			mmio_read_32(UNIPHIER_PARAM_BOOT_CONSOLE_PORT);
	bd->boot_console.baud_rate =
			mmio_read_32(UNIPHIER_PARAM_BOOT_CONSOLE_BAUDRATE);

	bd->runtime_console.uart_port =
			mmio_read_32(UNIPHIER_PARAM_RUNTIME_CONSOLE_PORT);
	bd->runtime_console.baud_rate =
			mmio_read_32(UNIPHIER_PARAM_RUNTIME_CONSOLE_BAUDRATE);
}

void uniphier_get_dram_data(struct uniphier_dram_data *dram)
{
	dram->board_type = mmio_read_32(UNIPHIER_PARAM_BOARD_TYPE);

	dram->dram_freq = mmio_read_32(UNIPHIER_PARAM_DRAM_FREQ);

	dram->dram_ch[0].base = mmio_read_64(UNIPHIER_PARAM_DRAM_CH0_BASE);
	dram->dram_ch[0].size = mmio_read_64(UNIPHIER_PARAM_DRAM_CH0_SIZE);
	dram->dram_ch[0].width = mmio_read_64(UNIPHIER_PARAM_DRAM_CH0_WIDTH);

	dram->dram_ch[1].base = mmio_read_64(UNIPHIER_PARAM_DRAM_CH1_BASE);
	dram->dram_ch[1].size = mmio_read_64(UNIPHIER_PARAM_DRAM_CH1_SIZE);
	dram->dram_ch[1].width = mmio_read_64(UNIPHIER_PARAM_DRAM_CH1_WIDTH);

	dram->dram_ch[2].base = mmio_read_64(UNIPHIER_PARAM_DRAM_CH2_BASE);
	dram->dram_ch[2].size = mmio_read_64(UNIPHIER_PARAM_DRAM_CH2_SIZE);
	dram->dram_ch[2].width = mmio_read_64(UNIPHIER_PARAM_DRAM_CH2_WIDTH);

	dram->dram_nr_ch = dram->dram_ch[2].size ? 3 : 2;
}

void uniphier_show_board_data(const struct uniphier_board_data *bd)
{
	/* show info */
	NOTICE("BOARD: %s\n", bd->board_name);
}

int uniphier_show_dram_data(const struct uniphier_dram_data *dram)
{
	int ch;

	/* show info */
	for (ch = 0; ch < dram->dram_nr_ch; ch++) {
		NOTICE("DRAM%d: 0x%lx-%lx (%dbit bus)\n", ch,
		     dram->dram_ch[ch].base,
		     dram->dram_ch[ch].base + dram->dram_ch[ch].size,
		     dram->dram_ch[ch].width);
	}

	/* sanity check */
	for (ch = 0; ch < dram->dram_nr_ch; ch++) {
		if (!dram->dram_ch[ch].size) {
			ERROR("DRAM ch%d size is zero\n", ch);
			return -EINVAL;
		}

		if (dram->dram_ch[ch].width != 16 &&
		    dram->dram_ch[ch].width != 32) {
			ERROR("DRAM ch%d size is zero\n", ch);
			return -EINVAL;
		}
	}

	for (ch = 0; ch < dram->dram_nr_ch - 1; ch++) {
		if (dram->dram_ch[ch].base + dram->dram_ch[ch].size >
		    dram->dram_ch[ch + 1].base) {
			ERROR("DRAM ch%d and ch%d overlap\n", ch, ch + 1);
			return -EINVAL;
		}
	}

	return 0;
}

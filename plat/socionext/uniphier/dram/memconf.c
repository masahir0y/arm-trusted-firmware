
#include <debug.h>
#include <mmio.h>
#include <plat_uniphier.h>

#define UNIPHIER_MEMCONF		0x5f800400

#define UNIPHIER_MEMCONF_CH0_SZ_MASK	((0x1 << 10) | (0x03 << 0))
#define UNIPHIER_MEMCONF_CH0_SZ_64M	((0x0 << 10) | (0x01 << 0))
#define UNIPHIER_MEMCONF_CH0_SZ_128M	((0x0 << 10) | (0x02 << 0))
#define UNIPHIER_MEMCONF_CH0_SZ_256M	((0x0 << 10) | (0x03 << 0))
#define UNIPHIER_MEMCONF_CH0_SZ_512M	((0x1 << 10) | (0x00 << 0))
#define UNIPHIER_MEMCONF_CH0_SZ_1G	((0x1 << 10) | (0x01 << 0))
#define UNIPHIER_MEMCONF_CH0_NUM_MASK	(0x1 << 8)
#define UNIPHIER_MEMCONF_CH0_NUM_1	(0x1 << 8)
#define UNIPHIER_MEMCONF_CH0_NUM_2	(0x0 << 8)

#define UNIPHIER_MEMCONF_CH1_SZ_MASK	((0x1 << 11) | (0x03 << 2))
#define UNIPHIER_MEMCONF_CH1_SZ_64M	((0x0 << 11) | (0x01 << 2))
#define UNIPHIER_MEMCONF_CH1_SZ_128M	((0x0 << 11) | (0x02 << 2))
#define UNIPHIER_MEMCONF_CH1_SZ_256M	((0x0 << 11) | (0x03 << 2))
#define UNIPHIER_MEMCONF_CH1_SZ_512M	((0x1 << 11) | (0x00 << 2))
#define UNIPHIER_MEMCONF_CH1_SZ_1G	((0x1 << 11) | (0x01 << 2))
#define UNIPHIER_MEMCONF_CH1_NUM_MASK	(0x1 << 9)
#define UNIPHIER_MEMCONF_CH1_NUM_1	(0x1 << 9)
#define UNIPHIER_MEMCONF_CH1_NUM_2	(0x0 << 9)

#define UNIPHIER_MEMCONF_CH2_SZ_MASK	((0x1 << 26) | (0x03 << 16))
#define UNIPHIER_MEMCONF_CH2_SZ_64M	((0x0 << 26) | (0x01 << 16))
#define UNIPHIER_MEMCONF_CH2_SZ_128M	((0x0 << 26) | (0x02 << 16))
#define UNIPHIER_MEMCONF_CH2_SZ_256M	((0x0 << 26) | (0x03 << 16))
#define UNIPHIER_MEMCONF_CH2_SZ_512M	((0x1 << 26) | (0x00 << 16))
#define UNIPHIER_MEMCONF_CH2_SZ_1G	((0x1 << 26) | (0x01 << 16))
#define UNIPHIER_MEMCONF_CH2_NUM_MASK	(0x1 << 24)
#define UNIPHIER_MEMCONF_CH2_NUM_1	(0x1 << 24)
#define UNIPHIER_MEMCONF_CH2_NUM_2	(0x0 << 24)

#define UNIPHIER_MEMCONF_CH2_DISABLE	(0x1 << 21)
#define UNIPHIER_MEMCONF_SPARSEMEM	(0x1 << 4)

static int __uniphier_memconf_init(const struct uniphier_dram_data *dram,
				   int have_ch2)
{
	uint32_t val = 0;
	unsigned long size_per_word;

	/* set up ch0 */
	switch (dram->dram_ch[0].width) {
	case 16:
		val |= UNIPHIER_MEMCONF_CH0_NUM_1;
		size_per_word = dram->dram_ch[0].size;
		break;
	case 32:
		val |= UNIPHIER_MEMCONF_CH0_NUM_2;
		size_per_word = dram->dram_ch[0].size >> 1;
		break;
	default:
		ERROR("memconf: unsupported DRAM ch0 width\n");
		return -EINVAL;
	}

	switch (size_per_word) {
	case SZ_64M:
		val |= UNIPHIER_MEMCONF_CH0_SZ_64M;
		break;
	case SZ_128M:
		val |= UNIPHIER_MEMCONF_CH0_SZ_128M;
		break;
	case SZ_256M:
		val |= UNIPHIER_MEMCONF_CH0_SZ_256M;
		break;
	case SZ_512M:
		val |= UNIPHIER_MEMCONF_CH0_SZ_512M;
		break;
	case SZ_1G:
		val |= UNIPHIER_MEMCONF_CH0_SZ_1G;
		break;
	default:
		ERROR("memconf: unsupported DRAM ch0 size\n");
		return -EINVAL;
	}

	/* set up ch1 */
	switch (dram->dram_ch[1].width) {
	case 16:
		val |= UNIPHIER_MEMCONF_CH1_NUM_1;
		size_per_word = dram->dram_ch[1].size;
		break;
	case 32:
		val |= UNIPHIER_MEMCONF_CH1_NUM_2;
		size_per_word = dram->dram_ch[1].size >> 1;
		break;
	default:
		ERROR("memconf: unsupported DRAM ch1 width\n");
		return -EINVAL;
	}

	switch (size_per_word) {
	case SZ_64M:
		val |= UNIPHIER_MEMCONF_CH1_SZ_64M;
		break;
	case SZ_128M:
		val |= UNIPHIER_MEMCONF_CH1_SZ_128M;
		break;
	case SZ_256M:
		val |= UNIPHIER_MEMCONF_CH1_SZ_256M;
		break;
	case SZ_512M:
		val |= UNIPHIER_MEMCONF_CH1_SZ_512M;
		break;
	case SZ_1G:
		val |= UNIPHIER_MEMCONF_CH1_SZ_1G;
		break;
	default:
		ERROR("memconf: unsupported DRAM ch1 size\n");
		return -EINVAL;
	}

	/* is sparse mem? */
	if (dram->dram_ch[0].base + dram->dram_ch[0].size <
	    dram->dram_ch[1].base)
		val |= UNIPHIER_MEMCONF_SPARSEMEM;

	if (!have_ch2)
		goto out;

	if (!dram->dram_ch[2].size) {
		val |= UNIPHIER_MEMCONF_CH2_DISABLE;
		goto out;
	}

	/* set up ch2 */
	switch (dram->dram_ch[2].width) {
	case 16:
		val |= UNIPHIER_MEMCONF_CH2_NUM_1;
		size_per_word = dram->dram_ch[2].size;
		break;
	case 32:
		val |= UNIPHIER_MEMCONF_CH2_NUM_2;
		size_per_word = dram->dram_ch[2].size >> 1;
		break;
	default:
		ERROR("memconf: unsupported DRAM ch2 width\n");
		return -EINVAL;
	}

	switch (size_per_word) {
	case SZ_64M:
		val |= UNIPHIER_MEMCONF_CH2_SZ_64M;
		break;
	case SZ_128M:
		val |= UNIPHIER_MEMCONF_CH2_SZ_128M;
		break;
	case SZ_256M:
		val |= UNIPHIER_MEMCONF_CH2_SZ_256M;
		break;
	case SZ_512M:
		val |= UNIPHIER_MEMCONF_CH2_SZ_512M;
		break;
	case SZ_1G:
		val |= UNIPHIER_MEMCONF_CH2_SZ_1G;
		break;
	default:
		ERROR("memconf: unsupported DRAM ch2 size\n");
		return -EINVAL;
	}

out:
	mmio_write_32(UNIPHIER_MEMCONF, val);

	return 0;
}

int uniphier_memconf_2ch_init(const struct uniphier_dram_data *dram)
{
	return __uniphier_memconf_init(dram, 0);
}

int uniphier_memconf_3ch_init(const struct uniphier_dram_data *dram)
{
	return __uniphier_memconf_init(dram, 1);
}

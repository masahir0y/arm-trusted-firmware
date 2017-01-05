#include <mmio.h>

#define SG_REVISION		0x5f800000
#define   SG_REVISION_TYPE_SHIFT	16
#define   SG_REVISION_TYPE_MASK		(0xff << SG_REVISION_TYPE_SHIFT)

unsigned int uniphier_get_soc_id(void)
{
	uint32_t rev = mmio_read_32(SG_REVISION);

	switch ((rev & SG_REVISION_TYPE_MASK) >> SG_REVISION_TYPE_SHIFT) {
	case 0x31:
		return UNIPHIER_SOC_LD11;
	case 0x32:
		return UNIPHIER_SOC_LD20;
	case 0x35:
		return UNIPHIER_SOC_PXS3;
	default:
		return UNIPHIER_SOC_UNKNOWN;
	}
}

#include <mmio.h>

#define SG_REVISION		0x5f800000
#define   SG_REVISION_TYPE_SHIFT	16
#define   SG_REVISION_TYPE_MASK		(0xff << SG_REVISION_TYPE_SHIFT)

unsigned int uniphier_get_soc_id(void)
{
	uint32_t rev = mmio_read_32(SG_REVISION);

	return (rev & SG_REVISION_TYPE_MASK) >> SG_REVISION_TYPE_SHIFT;
}

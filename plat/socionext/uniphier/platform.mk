#
# Copyright (c) 2013-2016, ARM Limited and Contributors. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# Neither the name of ARM nor the names of its contributors may be used
# to endorse or promote products derived from this software without specific
# prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

override COLD_BOOT_SINGLE_CPU	:= 1
override LOAD_IMAGE_V2		:= 1
# Disable the PSCI platform compatibility layer
override ENABLE_PLAT_COMPAT	:= 0
# UniPhier platform does not need deprecated functions
override ERROR_DEPRECATED	:= 1

override USE_COHERENT_MEM	:= 1

FIP_ALIGN			:= 512

BL1_PREPAD			:= 4096
CONFIG_UNIPHIER_LD11		:= y
CONFIG_UNIPHIER_LD20		:= y
CONFIG_UNIPHIER_PXS3		:= y

$(eval $(call add_define,BL1_PREPAD))

ifeq ($(CONFIG_UNIPHIER_LD11),y)
$(eval $(call add_define,CONFIG_UNIPHIER_LD11))
endif

ifeq ($(CONFIG_UNIPHIER_LD20),y)
$(eval $(call add_define,CONFIG_UNIPHIER_LD20))
endif

ifeq ($(CONFIG_UNIPHIER_PXS3),y)
$(eval $(call add_define,CONFIG_UNIPHIER_PXS3))
endif

PLAT_PATH			:= plat/socionext/uniphier
PLAT_INCLUDES			:= -I$(PLAT_PATH)/include

# IO sources for BL1, BL2
io-y				+= drivers/io/io_block.c
io-y				+= drivers/io/io_fip.c
io-y				+= drivers/io/io_memmap.c
io-y				+= drivers/io/io_storage.c
io-y				+= $(PLAT_PATH)/uniphier_boot_device.c
io-y				+= $(PLAT_PATH)/uniphier_io_storage.c
io-y				+= $(PLAT_PATH)/uniphier_rom_api.c

# xlat sources for BL2, BL31
xlat-y				+= lib/xlat_tables/aarch64/xlat_tables.c
xlat-y				+= lib/xlat_tables/xlat_tables_common.c
xlat-y				+= $(PLAT_PATH)/uniphier_xlat_setup.c

# common sources for BL1, BL2, BL31
common-y			+= drivers/console/aarch64/console.S
common-y			+= $(PLAT_PATH)/uniphier_clk.c
common-y			+= $(PLAT_PATH)/uniphier_console.S
common-y			+= $(PLAT_PATH)/uniphier_console_setup.c
common-y			+= $(PLAT_PATH)/uniphier_helpers.S
common-y			+= $(PLAT_PATH)/uniphier_pinctrl.c
common-y			+= $(PLAT_PATH)/uniphier_soc_info.c

# BL1 sources
bl1-$(CONFIG_UNIPHIER_LD11)	+= lib/cpus/aarch64/cortex_a53.S
bl1-$(CONFIG_UNIPHIER_LD20)	+= lib/cpus/aarch64/cortex_a72.S
bl1-$(CONFIG_UNIPHIER_PXS3)	+= lib/cpus/aarch64/cortex_a53.S
bl1-y				+= plat/common/aarch64/platform_up_stack.S
bl1-y				+= $(PLAT_PATH)/uniphier_bl1_helpers.S
bl1-y				+= $(PLAT_PATH)/uniphier_bl1_setup.c
bl1-y				+= $(PLAT_PATH)/uniphier_board_param.c
bl1-y				+= $(PLAT_PATH)/uniphier_soc_setup.c
bl1-y				+= $(PLAT_PATH)/dram/memconf.c
bl1-$(CONFIG_UNIPHIER_LD11)	+= $(PLAT_PATH)/dram/umc_ld11.c
bl1-$(CONFIG_UNIPHIER_LD20)	+= $(PLAT_PATH)/dram/umc_ld20.c
bl1-$(CONFIG_UNIPHIER_PXS3)	+= $(PLAT_PATH)/dram/umc_pxs3.c
bl1-y				+= $(io-y)

# BL2 sources
bl2-y				+= plat/common/aarch64/platform_up_stack.S
bl2-y				+= common/desc_image_load.c
bl2-y				+= $(PLAT_PATH)/uniphier_bl2_setup.c
bl2-y				+= $(PLAT_PATH)/uniphier_board_param.c
bl2-y				+= $(PLAT_PATH)/uniphier_image_desc.c
bl2-y				+= $(io-y)
bl2-y				+= $(xlat-y)

# BL31 sources
bl31-$(CONFIG_UNIPHIER_LD20)	+= drivers/arm/cci/cci.c
bl31-y				+= drivers/arm/gic/common/gic_common.c
bl31-y				+= drivers/arm/gic/v3/gicv3_helpers.c
bl31-y				+= drivers/arm/gic/v3/gicv3_main.c
bl31-y				+= lib/cpus/aarch64/cortex_a53.S
bl31-y				+= lib/cpus/aarch64/cortex_a72.S
bl31-y				+= plat/common/aarch64/plat_common.c
bl31-y				+= plat/common/aarch64/platform_mp_stack.S
bl31-y				+= plat/common/plat_gicv3.c
bl31-y				+= plat/common/plat_psci_common.c
bl31-y				+= $(PLAT_PATH)/uniphier_bl31_setup.c
bl31-$(CONFIG_UNIPHIER_LD20)	+= $(PLAT_PATH)/uniphier_cci.c
bl31-y				+= $(PLAT_PATH)/uniphier_gicv3.c
bl31-y				+= $(PLAT_PATH)/uniphier_pm.c
bl31-y				+= $(PLAT_PATH)/uniphier_smp.S
bl31-y				+= $(PLAT_PATH)/uniphier_syscnt.c
bl31-y				+= $(PLAT_PATH)/uniphier_topology.c
bl31-y				+= $(xlat-y)

PLAT_BL_COMMON_SOURCES		:= $(sort $(common-y))
BL1_SOURCES			:= $(sort $(bl1-y))
BL2_SOURCES			:= $(sort $(bl2-y))
BL31_SOURCES 			:= $(sort $(bl31-y))

param-$(CONFIG_UNIPHIER_LD11)	+= ld11*.mk
param-$(CONFIG_UNIPHIER_LD20)	+= ld20*.mk ld21*.mk ld22*.mk
param-$(CONFIG_UNIPHIER_PXS3)	+= pxs3*.mk

BOARD_PARAMS	:=  $(wildcard $(addprefix $(PLAT_PATH)/board_params/, $(param-y)))

.PHONY: uniphier_bl1

define MAKE_UNIPHIER_BL1

	$(eval BIN := $(call IMG_BIN,$(patsubst %.mk,1_%,$(notdir $(1)))))

$(BIN): $(BUILD_PLAT)/bl1.bin $(1)
	@echo "  GEN     $$@"
	$(Q)MAKEFLAGS="${MAKEFLAGS} --no-print-directory" \
	$${MAKE} -C $(PLAT_PATH)/board_params TARGET=$(CURDIR)/$$@ \
	PARAM_FILE=$(notdir $(1)) BL1_PREPAD=$(BL1_PREPAD) \
	BL1_BIN=$(CURDIR)/$(BUILD_PLAT)/bl1.bin

uniphier_bl1: $(BIN) bl1
endef

$(foreach b, $(BOARD_PARAMS), $(eval $(call MAKE_UNIPHIER_BL1,$(b))))

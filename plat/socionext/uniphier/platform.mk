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

# Disable the PSCI platform compatibility layer
override ENABLE_PLAT_COMPAT	:= 0
# UniPhier platform does not need deprecated functions
override ERROR_DEPRECATED	:= 1

override USE_COHERENT_MEM	:= 1

SUPPORTED_SOCS := ld11 ld20
ifneq ($(words $(filter $(SOC), $(SUPPORTED_SOCS))),1)
$(error Please specify SOC=<SOC>, here <SOC> is one of: $(SUPPORTED_SOCS))
endif

$(eval $(call add_define,UNIPHIER_$(call uppercase,$(SOC))))

PLAT_PATH		:=	plat/socionext/uniphier

PLAT_INCLUDES		:=	-I$(PLAT_PATH)/include

PLAT_BL_COMMON_SOURCES	+=	drivers/console/aarch64/console.S	\
				$(PLAT_PATH)/uniphier_common.c		\
				$(PLAT_PATH)/uniphier_console.S		\
				$(PLAT_PATH)/uniphier_helpers.S

BL31_SOURCES		+=	drivers/arm/cci/cci.c			\
				drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v3/gicv3_helpers.c	\
				drivers/arm/gic/v3/gicv3_main.c		\
				lib/cpus/aarch64/cortex_a53.S		\
				lib/cpus/aarch64/cortex_a72.S		\
				lib/xlat_tables/aarch64/xlat_tables.c	\
				lib/xlat_tables/xlat_tables_common.c	\
				plat/common/aarch64/plat_common.c	\
				plat/common/aarch64/platform_mp_stack.S	\
				plat/common/plat_gicv3.c		\
				plat/common/plat_psci_common.c		\
				$(PLAT_PATH)/uniphier_bl31_setup.c	\
				$(PLAT_PATH)/uniphier_cci.c		\
				$(PLAT_PATH)/uniphier_gicv3.c		\
				$(PLAT_PATH)/uniphier_pm.c		\
				$(PLAT_PATH)/uniphier_smp.S		\
				$(PLAT_PATH)/uniphier_topology.c

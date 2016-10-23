include $(PARAM_FILE)

# Do not use bash-builtin echo
ECHO = /bin/echo -n -e

define puts
	$(ECHO)	$1 >> $@
endef

define put8
	$(call puts, '\x'$$(printf '%x' $$(( ($1) ))))
endef

define put16
	$(call put8, ( ($1) >>  0 ) & 0xFF)
	$(call put8, ( ($1) >>  8 ) & 0xFF)
endef

define put32
	$(call put16, ( ($1) >>  0 ) & 0xFFFF)
	$(call put16, ( ($1) >> 16 ) & 0xFFFF)
endef

define put64
	$(call put32, ( ($1) >>  0 ) & 0xFFFFFFFF)
	$(call put32, ( ($1) >> 32 ) & 0xFFFFFFFF)
endef

define puts_len
	s=$$($(ECHO) "$1" | cut -c-$2);	\
	$(call puts,$$s);		\
	i=$$($(ECHO) $$s | wc -m);	\
	while [ $$i -lt $2 ];		\
	do				\
		$(call put8,0);		\
		i=$$(( $$i + 1 ));	\
	done
endef

define gen_sec0
#	branch instruction
	$(call put32, 0x14000000 | ($(BL1_PREPAD) >> 2))
endef

define gen_sec1
	$(call puts, UNPH)
	$(call put32, 0)
	$(call put64, 0)
	$(call puts_len,$(board-name),32)
	$(call put32, $(boot-console-port))
	$(call put32, $(boot-console-baudrate))
	$(call put32, $(runtime-console-port))
	$(call put32, $(runtime-console-baudrate))
	$(call put32, $(board-type))
	$(call put32, $(dram-freq))
	$(call put64, $(dram-ch0-base))
	$(call put64, $(dram-ch0-size))
	$(call put64, $(dram-ch0-width))
	$(call put64, $(dram-ch1-base))
	$(call put64, $(dram-ch1-size))
	$(call put64, $(dram-ch1-width))
	$(call put64, $(dram-ch2-base))
	$(call put64, $(dram-ch2-size))
	$(call put64, $(dram-ch2-width))
endef

define cmd_gen
	rm -f $@
	$(call gen_sec0)
	objcopy -I binary -O binary --pad-to=512 $@
	$(call gen_sec1)
	objcopy -I binary -O binary --pad-to=$(BL1_PREPAD) $@
	cat $(BL1_BIN) >> $@
endef

define check
	$(eval $(if $($1),, $(error "$1" is not defined)))
endef

define default_if_unset
	$(eval $(if $($1),,$1 := $2))
endef

$(call check,boot-console-port)
$(call check,boot-console-baudrate)
$(call default_if_unset,runtime-console-port,$(boot-console-port))
$(call default_if_unset,runtime-console-baudrate,$(boot-console-baudrate))
$(call check,board-type)
$(call check,dram-freq)
$(call check,dram-ch0-base)
$(call check,dram-ch0-size)
$(call check,dram-ch0-width)
$(call check,dram-ch1-base)
$(call check,dram-ch1-size)
$(call check,dram-ch1-width)
$(call check,dram-ch2-base)
$(call check,dram-ch2-size)
$(call check,dram-ch2-width)

$(TARGET): FORCE
	@$(call cmd_gen)

.PHONY: FORCE

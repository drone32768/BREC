include ../../common.mk

all	:
	@echo host is $(HOSTARCH)
	@echo target is $(TGT)
ifeq ($(TGT),arm)
	make brecA-00A0.dtbo
endif

brecA-00A0.dtbo	:	brecA-00A0.dts
	dtc -O dtb -o brecA-00A0.dtbo -b 0 -@ brecA-00A0.dts
	mv brecA-00A0.dtbo /lib/firmware

dt-show	:
	cat /sys/devices/bone_capemgr.8/slots

dt-install	: brecA-00A0.dtbo
	./dt-install

clean	:
	rm -f *.o
	rm -f *.bin
	rm -f *.dtbo
	rm -f *~
	rm -f tmp

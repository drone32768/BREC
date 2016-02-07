include ../../common.mk

all	:
	@echo host is $(HOSTARCH)
	@echo target is $(TGT)
ifeq ($(TGT),arm)
	make brecI-00A0.dtbo
endif

brecI-00A0.dtbo	:	brecI-00A0.dts
	dtc -O dtb -o brecI-00A0.dtbo -b 0 -@ brecI-00A0.dts
	mv brecI-00A0.dtbo /lib/firmware

dt-show	:
	cat /sys/devices/bone_capemgr.8/slots

dt-install	: brecI-00A0.dtbo
	./dt-install

clean	:
	rm -f *.o
	rm -f *.bin
	rm -f *.dtbo
	rm -f *~
	rm -f tmp

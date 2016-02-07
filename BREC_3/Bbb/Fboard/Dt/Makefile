include ../../common.mk

all	:
	@echo host is $(HOSTARCH)
	@echo target is $(TGT)
ifeq ($(TGT),arm)
	make /lib/firmware/brecFjtag-00A0.dtbo
	make /lib/firmware/brecFpru-00A0.dtbo
endif

/lib/firmware/brecFpru-00A0.dtbo	:	brecFpru-00A0.dts
	dtc -O dtb -o brecFpru-00A0.dtbo -b 0 -@ brecFpru-00A0.dts
	mv brecFpru-00A0.dtbo /lib/firmware

/lib/firmware/brecFjtag-00A0.dtbo	:	brecFjtag-00A0.dts
	dtc -O dtb -o brecFjtag-00A0.dtbo -b 0 -@ brecFjtag-00A0.dts
	mv brecFjtag-00A0.dtbo /lib/firmware

dt-show	:
	cat /sys/devices/bone_capemgr.8/slots

dt-install	: brecF-00A0.dtbo
	./dt-install

clean	:
	rm /lib/firmware/brecFpru-00A0.dtbo
	rm /lib/firmware/brecFjtag-00A0.dtbo
	rm -f *.o
	rm -f *.bin
	rm -f *.dtbo
	rm -f *~
	rm -f tmp

include ../../common.mk

all	:
	@echo host is $(HOSTARCH)
	@echo target is $(TGT)
ifeq ($(TGT),arm)
	make /lib/firmware/brecH-00A0.dtbo
endif

/lib/firmware/brecH-00A0.dtbo	:	brecH-00A0.dts
	dtc -O dtb -o brecH-00A0.dtbo -b 0 -@ brecH-00A0.dts
	mv brecH-00A0.dtbo /lib/firmware

dt-show	:
	cat /sys/devices/bone_capemgr.8/slots

dt-install	: brecH-00A0.dtbo
	./dt-install

clean	:
	rm /lib/firmware/brecH-00A0.dtbo
	rm -f *.o
	rm -f *.bin
	rm -f *.dtbo
	rm -f *~
	rm -f tmp

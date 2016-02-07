include ../../common.mk

all	:
	@echo host is $(HOSTARCH)
	@echo target is $(TGT)
ifeq ($(TGT),arm)
	make /lib/firmware/brecX-00A0.dtbo
endif

/lib/firmware/brecX-00A0.dtbo	:	brecX-00A0.dts
	dtc -O dtb -o brecX-00A0.dtbo -b 0 -@ brecX-00A0.dts
	mv brecX-00A0.dtbo /lib/firmware

dt-show	:
	cat /sys/devices/bone_capemgr.8/slots

dt-install	: brecX-00A0.dtbo
	./dt-install

clean	:
	rm /lib/firmware/brecX-00A0.dtbo
	rm -f *.o
	rm -f *.bin
	rm -f *.dtbo
	rm -f *~
	rm -f tmp

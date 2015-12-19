include ../../common.mk

all	:
	@echo host is $(HOSTARCH)
	@echo target is $(TGT)
ifeq ($(TGT),arm)
	make /lib/firmware/brecJtag-00A0.dtbo
	make /lib/firmware/brecF-00A0.dtbo
endif

/lib/firmware/brecF-00A0.dtbo	:	brecF-00A0.dts
	dtc -O dtb -o brecF-00A0.dtbo -b 0 -@ brecF-00A0.dts
	mv brecF-00A0.dtbo /lib/firmware

/lib/firmware/brecJtag-00A0.dtbo	:	brecJtag-00A0.dts
	dtc -O dtb -o brecJtag-00A0.dtbo -b 0 -@ brecJtag-00A0.dts
	mv brecJtag-00A0.dtbo /lib/firmware

dt-show	:
	cat /sys/devices/bone_capemgr.8/slots

dt-install	: brecF-00A0.dtbo
	./dt-install

clean	:
	rm /lib/firmware/brecF-00A0.dtbo
	rm /lib/firmware/brecJtag-00A0.dtbo
	rm -f *.o
	rm -f *.bin
	rm -f *.dtbo
	rm -f *~
	rm -f tmp

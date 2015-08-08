include ../common.mk

ifeq ($(TGT),arm)
    PRU_INC=/home/root/am335x_pru_package-master/pru_sw/app_loader/include
    PRU_LIB=/home/root/am335x_pru_package-master/pru_sw/app_loader/lib
    PASM=/home/root/am335x_pru_package-master/pru_sw/utils/pasm
else
    PRU_INC=../PruEmul/
endif

UTIL_OBJS=../Util/$(TGT)/gpioutil.o ../Util/$(TGT)/mcf.o ../Iboard/$(TGT)/Iboard.o
DEV_OBJS=
LOCAL_OBJS=

all	:
	make $(TGT)/Xboard.o
	make $(TGT)/Xutil
ifeq ($(TGT),arm)
	make $(TGT)/pru00.bin
	make $(TGT)/pru01.bin
	make $(TGT)/pructl
	cd Dt; make all
endif


$(TGT)/Xboard.o    : Xboard.cpp $(LOCAL_OBJS) $(UTIL_OBJS) $(DEV_OBJS)  | $(TGT)
	g++ -c $(CPPFLAGS) Xboard.cpp -o $(TGT)/Xboard.o

$(TGT)/Xutil    : Xutil.cpp $(LOCAL_OBJS) $(UTIL_OBJS) $(DEV_OBJS)  | $(TGT)
	g++ $(CPPFLAGS) Xutil.cpp \
                $(LOCAL_OBJS) \
                $(DEV_OBJS) $(UTIL_OBJS) \
                -lm -lpthread -o $(TGT)/Xutil
$(TGT)/pru00.bin : pru00.p Xpru.h pru00.hp | $(TGT)
	${PASM} -V3 -b pru00.p
	mv pru00.bin $(TGT)/pru00.bin

$(TGT)/pru01.bin : pru01.p Xpru.h pru00.hp | $(TGT)
	${PASM} -V3 -b pru01.p
	mv pru01.bin $(TGT)/pru01.bin

ifeq ($(TGT),arm)
pru_images.h    : $(TGT)/pru00.bin $(TGT)/pru01.bin | $(TGT)
	../Util/$(TGT)/ccat -f $(TGT)/pru00.bin -n pru_image0 >  pru_images.h
	../Util/$(TGT)/ccat -f $(TGT)/pru01.bin -n pru_image1 >> pru_images.h
endif

clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp

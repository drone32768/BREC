include ../common.mk

ifeq ($(TGT),arm)
    PRU_INC=/home/root/am335x_pru_package-master/pru_sw/app_loader/include
    PRU_LIB=/home/root/am335x_pru_package-master/pru_sw/app_loader/lib
    PASM=/home/root/am335x_pru_package-master/pru_sw/utils/pasm
else
    PRU_INC=../PruEmul/
    PRU_LIB=../PruEmul/x86
endif

UTIL_OBJS=../Fboard/$(TGT)/Fboard.o \
          ../Util/$(TGT)/mcf.o \
          ../Util/$(TGT)/gpioutil.o
LOCAL_OBJS=$(TGT)/Ddc100.o


all	:
	make $(TGT)/DdcTst

$(TGT)/DdcTst	: DdcTst.cpp $(LOCAL_OBJS) | $(TGT)
	g++ -I../ DdcTst.cpp \
             $(LOCAL_OBJS) \
             $(UTIL_OBJS) \
             -L$(PRU_LIB) \
             -lfftw3 -lprussdrv -lpthread -o $(TGT)/DdcTst

$(TGT)/Ddc100.o	: Ddc100.cpp Ddc100.h pru_images.h | $(TGT)
	g++ -c -I../ -I$(PRU_INC) Ddc100.cpp -o $(TGT)/Ddc100.o


$(TGT)/pru01.bin : pru01.p pruinc.h | $(TGT)
	${PASM} -V3 -b pru01.p
	mv pru01.bin $(TGT)/pru01.bin

ifeq ($(TGT),arm)
pru_images.h    : $(TGT)/pru01.bin | $(TGT)
	../Util/$(TGT)/ccat -f $(TGT)/pru01.bin -n pru_image01 >  pru_images.h
endif


clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp


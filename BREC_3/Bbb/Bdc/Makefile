include ../MakeCommon.mk

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
LOCAL_OBJS=$(TGT)/Bdc.o


all	:
	make $(TGT)/BdcTst

$(TGT)/BdcTst	: BdcTst.cpp $(LOCAL_OBJS) | $(TGT)
	g++ -I../ BdcTst.cpp \
             $(LOCAL_OBJS) \
             $(UTIL_OBJS) \
             -L$(PRU_LIB) \
             -lfftw3 -lprussdrv -lpthread -o $(TGT)/BdcTst

$(TGT)/Bdc.o	: Bdc.cpp Bdc.h | $(TGT)
	g++ -c -I../ -I$(PRU_INC) Bdc.cpp -o $(TGT)/Bdc.o


clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp


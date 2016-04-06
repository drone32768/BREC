include ../MakeCommon.mk

ifeq ($(TGT),arm)
    PRU_INC=/home/root/am335x_pru_package-master/pru_sw/app_loader/include
    PRU_LIB=/home/root/am335x_pru_package-master/pru_sw/app_loader/lib
    PASM=/home/root/am335x_pru_package-master/pru_sw/utils/pasm
else
    PRU_INC=../PruEmul/
    PRU_LIB=../PruEmul/x86
endif

UTIL_OBJS=../Util/$(TGT)/mcf.o \
          ../Util/$(TGT)/cli.o \
          ../Util/$(TGT)/net.o \
          ../Util/$(TGT)/gpioutil.o \
          ../Adf4351/$(TGT)/Adf4351.o \
          ../Iboard/$(TGT)/Iboard.o \
          ../Bdc/$(TGT)/Bdc.o \
          ../Fboard/$(TGT)/Fboard.o 

LOCAL_OBJS=$(TGT)/Mboard.o 

all	:
	make $(TGT)/Mboard.o
	make $(TGT)/Mutil

$(TGT)/Mboard.o : Mboard.cpp Mboard.h $(UTIL_OBJS) | $(TGT)
	g++ -I../ $(CPPFLAGS) -c Mboard.cpp -o $(TGT)/Mboard.o

$(TGT)/Mutil    : Mutil.cpp $(LOCAL_OBJS) $(UTIL_OBJS) $(DEV_OBJS)  | $(TGT)
	g++ -I../ $(CPPFLAGS) Mutil.cpp \
		$(LOCAL_OBJS) \
                $(UTIL_OBJS) \
                -L$(PRU_LIB) \
                -lm -lprussdrv -lpthread -o $(TGT)/Mutil

clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp

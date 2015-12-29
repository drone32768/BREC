include ../common.mk

ifeq ($(TGT),arm)
    PRU_INC=/home/root/am335x_pru_package-master/pru_sw/app_loader/include
    PRU_LIB=/home/root/am335x_pru_package-master/pru_sw/app_loader/lib
    PASM=/home/root/am335x_pru_package-master/pru_sw/utils/pasm
else
    PRU_INC=../PruEmul/
endif

UTIL_OBJS=../Util/$(TGT)/gpioutil.o ../Util/$(TGT)/mcf.o 
DEV_OBJS=
LOCAL_OBJS=$(TGT)/Fboard.o


all	:
	make $(TGT)/xvcSrvrSim
	make $(TGT)/xvcSrvrF
	make $(TGT)/Fctl
	make $(TGT)/Ftst
	make $(TGT)/Flash
ifeq ($(TGT),arm)
endif

$(TGT)/Fctl	: Fctl.cpp | $(TGT)
	g++ Fctl.cpp \
             $(UTIL_OBJS) \
             -lpthread -o $(TGT)/Fctl

$(TGT)/Ftst	: Ftst.cpp | $(TGT)
	g++ Ftst.cpp \
             $(UTIL_OBJS) \
             -lpthread -o $(TGT)/Ftst

$(TGT)/Flash	: Flash.cpp | $(TGT)
	g++ Flash.cpp \
             $(UTIL_OBJS) \
             -lpthread -o $(TGT)/Flash

$(TGT)/xvcSrvrSim	: $(TGT)/xvcSrvr.o $(TGT)/xvcSrvrGpioSim.o | $(TGT)
	g++ $(TGT)/xvcSrvr.o $(TGT)/xvcSrvrGpioSim.o -o $(TGT)/xvcSrvrSim

$(TGT)/xvcSrvrF	: $(TGT)/xvcSrvr.o $(TGT)/xvcSrvrGpioF.o | $(TGT)
	g++ $(TGT)/xvcSrvr.o $(TGT)/xvcSrvrGpioF.o \
            $(UTIL_OBJS) \
            -lpthread -o $(TGT)/xvcSrvrF

$(TGT)/xvcSrvr.o	: xvcSrvr.cpp xvcSrvrGpio.h | $(TGT)
	gcc -c -O3 xvcSrvr.cpp -o $(TGT)/xvcSrvr.o

$(TGT)/xvcSrvrGpioSim.o	: xvcSrvrGpioSim.cpp xvcSrvrGpio.h | $(TGT)
	gcc -c xvcSrvrGpioSim.cpp -o $(TGT)/xvcSrvrGpioSim.o 

$(TGT)/xvcSrvrGpioF.o	: xvcSrvrGpioF.cpp xvcSrvrGpio.h | $(TGT)
	g++ -c -O3 xvcSrvrGpioF.cpp -o $(TGT)/xvcSrvrGpioF.o 

clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp


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
	make $(TGT)/Fxsvf
	make $(TGT)/Fxvc
	make $(TGT)/Fctl
	make $(TGT)/Ftst
	make $(TGT)/Fflash
ifeq ($(TGT),arm)
endif

$(TGT)/Fctl	: Fctl.cpp $(LOCAL_OBJS) | $(TGT)
	g++ Fctl.cpp \
             $(LOCAL_OBJS) \
             $(UTIL_OBJS) \
             -lpthread -o $(TGT)/Fctl

$(TGT)/Ftst	: Ftst.cpp | $(TGT)
	g++ Ftst.cpp \
             $(UTIL_OBJS) \
             -lpthread -o $(TGT)/Ftst

$(TGT)/Fflash	: Fflash.cpp $(LOCAL_OBJS) | $(TGT)
	g++ Fflash.cpp \
             $(LOCAL_OBJS) \
             $(UTIL_OBJS) \
             -lpthread -o $(TGT)/Fflash

$(TGT)/Fxvc	: Fxvc.cpp $(TGT)/jtag_bs_f.o  | $(TGT)
	g++ Fxvc.cpp $(TGT)/jtag_bs_f.o \
            ../JtagTools/$(TGT)/xvcLib.o \
            $(UTIL_OBJS) \
            -lpthread -o $(TGT)/Fxvc

$(TGT)/Fxsvf	: Fxsvf.cpp $(TGT)/jtag_bs_f.o  | $(TGT)
	g++ Fxsvf.cpp $(TGT)/jtag_bs_f.o \
            ../JtagTools/$(TGT)/xsvfLib.o \
            $(UTIL_OBJS) \
            -lpthread -o $(TGT)/Fxsvf

$(TGT)/jtag_bs_f.o	: jtag_bs_f.cpp | $(TGT)
	g++ -c -O3 jtag_bs_f.cpp -o $(TGT)/jtag_bs_f.o

$(TGT)/Fboard.o	: Fboard.cpp | $(TGT)
	g++ -c Fboard.cpp -o $(TGT)/Fboard.o

clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp


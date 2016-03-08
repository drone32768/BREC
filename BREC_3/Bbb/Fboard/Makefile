include ../MakeCommon.mk

ifeq ($(TGT),arm)
    PRU_INC=/home/root/am335x_pru_package-master/pru_sw/app_loader/include
    PRU_LIB=/home/root/am335x_pru_package-master/pru_sw/app_loader/lib
    PASM=/home/root/am335x_pru_package-master/pru_sw/utils/pasm
else
    PRU_INC=../PruEmul/
    PRU_LIB=../PruEmul/x86
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

# 
# JTAG only targets
# 
$(TGT)/Fxvc	: Fxvc.cpp $(TGT)/jtag_bs_f.o  | $(TGT)
	g++ -I../ Fxvc.cpp $(TGT)/jtag_bs_f.o \
            ../JtagTools/$(TGT)/xvcLib.o \
            $(UTIL_OBJS) \
            -lpthread -o $(TGT)/Fxvc

$(TGT)/Fxsvf	: Fxsvf.cpp $(TGT)/jtag_bs_f.o  | $(TGT)
	g++ -I../ Fxsvf.cpp $(TGT)/jtag_bs_f.o \
            ../JtagTools/$(TGT)/xsvfLib.o \
            $(UTIL_OBJS) \
            -lpthread -o $(TGT)/Fxsvf

$(TGT)/jtag_bs_f.o	: jtag_bs_f.cpp | $(TGT)
	g++ -I../ -c -O3 jtag_bs_f.cpp -o $(TGT)/jtag_bs_f.o

# 
# HSPI only targets
# 
$(TGT)/Fctl	: Fctl.cpp $(LOCAL_OBJS) | $(TGT)
	g++ -I../ Fctl.cpp \
             $(LOCAL_OBJS) \
             $(UTIL_OBJS) \
             -L$(PRU_LIB) \
             -lprussdrv -lpthread -o $(TGT)/Fctl

$(TGT)/Ftst	: Ftst.cpp $(LOCAL_OBJS) | $(TGT)
	g++ -I../ Ftst.cpp \
             $(LOCAL_OBJS) \
             $(UTIL_OBJS) \
             -L$(PRU_LIB) \
             -lprussdrv -lpthread -o $(TGT)/Ftst

$(TGT)/Fflash	: Fflash.cpp $(LOCAL_OBJS) | $(TGT)
	g++ -I../ Fflash.cpp \
             $(LOCAL_OBJS) \
             $(UTIL_OBJS) \
             -L$(PRU_LIB) \
             -lprussdrv -lpthread -o $(TGT)/Fflash

$(TGT)/Fboard.o	: Fboard.cpp Fboard.h pru_images.h | $(TGT)
	g++ -I../ -c -I$(PRU_INC) Fboard.cpp -o $(TGT)/Fboard.o

$(TGT)/pru00.bin : pru00.p pruinc.h pruconst.hp | $(TGT)
	${PASM} -V3 -b pru00.p
	mv pru00.bin $(TGT)/pru00.bin

ifeq ($(TGT),arm)
pru_images.h    : $(TGT)/pru00.bin | $(TGT)
	../Util/$(TGT)/ccat -f $(TGT)/pru00.bin -n pru_image0 >  pru_images.h
endif

clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp


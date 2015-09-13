
include ../common.mk

ifeq ($(TGT),arm)
    PRU_INC=/home/root/am335x_pru_package-master/pru_sw/app_loader/include
    PRU_LIB=/home/root/am335x_pru_package-master/pru_sw/app_loader/lib
    PASM=/home/root/am335x_pru_package-master/pru_sw/utils/pasm
else
    PRU_INC=../PruEmul/
endif

UTIL_OBJS=../Util/$(TGT)/mcf.o           \
          ../Util/$(TGT)/gpioutil.o      \
          ../Util/$(TGT)/SimpleTable.o
DEV_OBJS=../Iboard/$(TGT)/Iboard.o       
LOCAL_OBJS= $(TGT)/Devs.o

all	:
	make $(TGT)/AdcHttpd

$(TGT)/AdcHttpd  : AdcHttpd.cpp $(LOCAL_OBJS) $(UTIL_OBJS) $(DEV_OBJS)  | $(TGT)
	g++ $(CPPFLAGS) AdcHttpd.cpp \
              -I$(PRU_INC)    \
                $(LOCAL_OBJS) \
                $(DEV_OBJS)   \
                $(UTIL_OBJS)  \
                -lm -lpthread -lmicrohttpd \
                -o $(TGT)/AdcHttpd

$(TGT)/Devs.o	: Devs.cpp Devs.h | $(TGT)
	g++ $(CPP_TGT_ARGS) $(CPPFLAGS) -c Devs.cpp  -o $(TGT)/Devs.o

clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp


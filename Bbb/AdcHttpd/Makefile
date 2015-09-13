
include ../common.mk

ifeq ($(TGT),arm)
    PRU_INC=/home/root/am335x_pru_package-master/pru_sw/app_loader/include
    PRU_LIB=/home/root/am335x_pru_package-master/pru_sw/app_loader/lib
    PASM=/home/root/am335x_pru_package-master/pru_sw/utils/pasm
else
    PRU_INC=../PruEmul/
endif

ifeq ($(TGT),arm)
else
    EMUL_OBJS=../PruEmul/$(TGT)/SignalSim.o ../PruEmul/$(TGT)/pruss_emul.o
endif

UTIL_OBJS=../Util/$(TGT)/mcf.o           \
          ../Util/$(TGT)/gpioutil.o      \
          ../Util/$(TGT)/SimpleTable.o
DEV_OBJS=../Iboard/$(TGT)/Iboard.o       \
         ../Xboard/$(TGT)/Xboard.o
LOCAL_OBJS= $(TGT)/Devs.o                \
            $(TGT)/Pse.o

all	:
	make $(TGT)/AdcHttpd

$(TGT)/AdcHttpd  : AdcHttpd.cpp $(LOCAL_OBJS) $(UTIL_OBJS) $(DEV_OBJS)  | $(TGT)
	g++ $(CPPFLAGS) AdcHttpd.cpp \
              -I$(PRU_INC)    \
                $(LOCAL_OBJS) \
                $(DEV_OBJS)   \
                $(UTIL_OBJS)  \
                $(EMUL_OBJS)  \
                -lm -lpthread -lmicrohttpd \
                -o $(TGT)/AdcHttpd

$(TGT)/Devs.o	: Devs.cpp Devs.h | $(TGT)
	g++ $(CPP_TGT_ARGS) $(CPPFLAGS) -c Devs.cpp  -o $(TGT)/Devs.o

$(TGT)/Pse.o	: Pse.cpp Pse.h | $(TGT)
	g++ $(CPP_TGT_ARGS) $(CPPFLAGS) -c Pse.cpp  -o $(TGT)/Pse.o

clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp


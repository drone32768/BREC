include ../common.mk

PRU_LIB=/home/root/am335x_pru_package-master/pru_sw/app_loader/lib
PRU_INC=/home/root/am335x_pru_package-master/pru_sw/app_loader/include 

UTIL_OBJS=../Util/$(TGT)/mcf.o ../Util/$(TGT)/cli.o ../Util/$(TGT)/net.o ../Util/$(TGT)/gpioutil.o
DEV_OBJS=../Aboard/$(TGT)/Aboard.o ../Bboard/$(TGT)/Bboard.o ../Adf4351/$(TGT)/Adf4351.o ../Rboard/$(TGT)/Rboard.o ../Hboard/$(TGT)/Hboard.o ../Iboard/$(TGT)/Iboard.o ../Mboard/$(TGT)/Mboard.o
EMUL_OBJS=../PruEmul/$(TGT)/SignalSim.o ../PruEmul/$(TGT)/pruss_emul.o
LOCAL_OBJS= $(TGT)/AscpUtil.o $(TGT)/AscpCtlIf.o $(TGT)/AscpDatIf.o \
            $(TGT)/AscpDisIf.o $(TGT)/CliCtlIf.o $(TGT)/Fir.o $(TGT)/Device.o 

all	:
ifeq ($(TGT),arm)
	make $(TGT)/SdrSvr
endif
	make $(TGT)/SdrSvrEmul

$(TGT)/CliCtlIf.o	: CliCtlIf.h CliCtlIf.cpp  | $(TGT)
	g++ $(CPPFLAGS) -c CliCtlIf.cpp -o $(TGT)/CliCtlIf.o

$(TGT)/AscpUtil.o	: AscpUtil.h AscpUtil.cpp  | $(TGT)
	g++ $(CPPFLAGS) -c AscpUtil.cpp -o $(TGT)/AscpUtil.o

$(TGT)/AscpDatIf.o	: AscpDatIf.h AscpDatIf.cpp  | $(TGT)
	g++ $(CPPFLAGS) -c AscpDatIf.cpp -o $(TGT)/AscpDatIf.o

$(TGT)/AscpCtlIf.o	: AscpCtlIf.h AscpCtlIf.cpp  | $(TGT)
	g++ $(CPPFLAGS) -c AscpCtlIf.cpp -o $(TGT)/AscpCtlIf.o

$(TGT)/AscpDisIf.o	: AscpDisIf.h AscpDisIf.cpp  | $(TGT)
	g++ $(CPPFLAGS) -c AscpDisIf.cpp -o $(TGT)/AscpDisIf.o

$(TGT)/Fir.o	: Fir.h Fir.cpp   | $(TGT)
	g++ $(CPPFLAGS) -c Fir.cpp -o $(TGT)/Fir.o

$(TGT)/Device.o	: Device.h Device.cpp Fir.h  | $(TGT)
	g++ -I$(PRU_INC) -I$(EMUL_INC1) $(CPPFLAGS) -c Device.cpp  \
                -o $(TGT)/Device.o

$(TGT)/SdrSvrEmul	: SdrSvr.cpp $(LOCAL_OBJS)\
                           $(EMUL_OBJS) $(DEV_OBJS) $(UTIL_OBJS)  | $(TGT)
	g++ $(CPPFLAGS) SdrSvr.cpp \
                $(LOCAL_OBJS) \
                $(DEV_OBJS) $(UTIL_OBJS)\
                $(EMUL_OBJS) \
	       	-o $(TGT)/SdrSvrEmul -lm -lpthread

$(TGT)/SdrSvr 	: SdrSvr.cpp  $(LOCAL_OBJS)\
                           $(EMUL_OBJS) $(DEV_OBJS) $(UTIL_OBJS)  | $(TGT)
	g++ $(CPPFLAGS) -I$(PRU_INC) -L$(PRU_LIB) SdrSvr.cpp \
                $(LOCAL_OBJS) \
                $(DEV_OBJS) $(UTIL_OBJS)\
	       	-o $(TGT)/SdrSvr -lm -lprussdrv -lpthread 

clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp

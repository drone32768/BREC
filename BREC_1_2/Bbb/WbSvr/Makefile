include ../common.mk


PRU_LIB=/home/root/am335x_pru_package-master/pru_sw/app_loader/lib
UTIL_OBJS=../Util/$(TGT)/mcf.o ../Util/$(TGT)/cli.o ../Util/$(TGT)/net.o ../Util/$(TGT)/gpioutil.o
DEV_OBJS=../Hboard/$(TGT)/Hboard.o ../Aboard/$(TGT)/Aboard.o ../Bboard/$(TGT)/Bboard.o ../Adf4351/$(TGT)/Adf4351.o ../Rboard/$(TGT)/Rboard.o ../Iboard/$(TGT)/Iboard.o ../Mboard/$(TGT)/Mboard.o ../Xboard/$(TGT)/Xboard.o
EMUL_OBJS=../PruEmul/$(TGT)/SignalSim.o ../PruEmul/$(TGT)/pruss_emul.o

LOCAL_OBJS=$(TGT)/WbCtlIf.o  $(TGT)/WbTests.o   \
           $(TGT)/AdcCtlIf.o $(TGT)/AdcDatIf.o  $(TGT)/AdcTcpIf.o \
           $(TGT)/RemoteDev.o     $(TGT)/SiEstimate.o\
           $(TGT)/SiCal.o    \
           $(TGT)/SiCtlIf.o  $(TGT)/SgCtlIf.o \
           $(TGT)/Devs.o

# Override CPP
CPPFLAGS=-g

all	:
ifeq ($(TGT),arm)
	make $(TGT)/WbSvr
endif
	make $(TGT)/WbSvrEmul
	make $(TGT)/TstEst

$(TGT)/WbCtlIf.o	: WbCtlIf.cpp WbCtlIf.h | $(TGT)
	g++ $(CPP_TGT_ARGS) $(CPPFLAGS) -c WbCtlIf.cpp  -o $(TGT)/WbCtlIf.o

$(TGT)/WbTests.o	: WbTests.cpp WbTests.h | $(TGT)
	g++ $(CPP_TGT_ARGS) $(CPPFLAGS) -c WbTests.cpp  -o $(TGT)/WbTests.o

$(TGT)/AdcCtlIf.o	: AdcCtlIf.cpp AdcCtlIf.h | $(TGT)
	g++ $(CPP_TGT_ARGS) $(CPPFLAGS) -c AdcCtlIf.cpp  -o $(TGT)/AdcCtlIf.o

$(TGT)/AdcDatIf.o	: AdcDatIf.cpp AdcDatIf.h | $(TGT)
	g++ $(CPP_TGT_ARGS) $(CPPFLAGS) -c AdcDatIf.cpp  -o $(TGT)/AdcDatIf.o

$(TGT)/AdcTcpIf.o	: AdcTcpIf.cpp AdcTcpIf.h | $(TGT)
	g++ $(CPP_TGT_ARGS) $(CPPFLAGS) -c AdcTcpIf.cpp  -o $(TGT)/AdcTcpIf.o

$(TGT)/SiCtlIf.o	: SiCtlIf.cpp SiCtlIf.h | $(TGT)
	g++ $(CPP_TGT_ARGS) $(CPPFLAGS) -c SiCtlIf.cpp  -o $(TGT)/SiCtlIf.o

$(TGT)/SiEstimate.o	: SiEstimate.cpp SiEstimate.h | $(TGT)
	g++ $(CPP_TGT_ARGS) $(CPPFLAGS) -c SiEstimate.cpp -o $(TGT)/SiEstimate.o

$(TGT)/RemoteDev.o	: RemoteDev.cpp RemoteDev.h | $(TGT)
	g++ $(CPP_TGT_ARGS) $(CPPFLAGS) -c RemoteDev.cpp  -o $(TGT)/RemoteDev.o

$(TGT)/SiCal.o	: SiCal.cpp SiCal.h | $(TGT)
	g++ $(CPP_TGT_ARGS) $(CPPFLAGS) -c SiCal.cpp  -o $(TGT)/SiCal.o

$(TGT)/SgCtlIf.o	: SgCtlIf.cpp SgCtlIf.h | $(TGT)
	g++ $(CPP_TGT_ARGS) $(CPPFLAGS) -c SgCtlIf.cpp  -o $(TGT)/SgCtlIf.o

$(TGT)/Devs.o	: Devs.cpp Devs.h | $(TGT)
	g++ $(CPP_TGT_ARGS) $(CPPFLAGS) -c Devs.cpp  -o $(TGT)/Devs.o

$(TGT)/WbSvr	: WbSvr.cpp $(LOCAL_OBJS) $(UTIL_OBJS) $(DEV_OBJS)  | $(TGT)
	g++ $(CPP_TGT_ARGS) $(CPPFLAGS) -L$(PRU_LIB) WbSvr.cpp \
                $(LOCAL_OBJS) \
		$(DEV_OBJS) $(UTIL_OBJS) \
                -lm -lfftw3 -lprussdrv -lpthread -o $(TGT)/WbSvr

$(TGT)/WbSvrEmul: WbSvr.cpp $(LOCAL_OBJS) $(UTIL_OBJS) $(DEV_OBJS) $(EMUL_OBJS) | $(TGT)
	g++ $(CPP_TGT_ARGS) WbSvr.cpp \
                $(LOCAL_OBJS) \
		$(EMUL_OBJS) $(DEV_OBJS) $(UTIL_OBJS) \
                -lm -lfftw3 -lpthread -o $(TGT)/WbSvrEmul

$(TGT)/TstEst	: TstEst.cpp $(TGT)/SiEstimate.o  | $(TGT)
	g++ $(CPP_TGT_ARGS) $(CPPFLAGS) TstEst.cpp $(TGT)/SiEstimate.o \
                $(UTIL_OBJS) \
                -lm -lfftw3 -lpthread -o $(TGT)/TstEst
clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp


include ../MakeCommon.mk

ifeq ($(TGT),arm)
    PRU_INC=/home/root/am335x_pru_package-master/pru_sw/app_loader/include
    PRU_LIB=/home/root/am335x_pru_package-master/pru_sw/app_loader/lib
    PASM=/home/root/am335x_pru_package-master/pru_sw/utils/pasm
    LIBS=-lprussdrv
else
    PRU_INC=../PruEmul/
    PRU_LIB=./
    LIBS=
endif

ifeq ($(TGT),arm)
    EMUL_OBJS=
else
    EMUL_OBJS=../PruEmul/$(TGT)/pruss_emul.o
endif

UTIL_OBJS=../Util/$(TGT)/mcf.o           \
          ../Util/$(TGT)/gpioutil.o      \
          ../Util/$(TGT)/SimpleTable.o

DEV_OBJS= \
         ../Fboard/$(TGT)/Fboard.o       \
         ../Bdc/$(TGT)/Bdc.o             \
         ../Ddc100/$(TGT)/Ddc100.o       \

LOCAL_OBJS= $(TGT)/Devs.o                \
            $(TGT)/Tok.o                 \
            $(TGT)/Pse.o                 \
            $(TGT)/His.o                 \
            $(TGT)/Tim.o

all	:
	make $(TGT)/AdcHttpd

$(TGT)/AdcHttpd  : AdcHttpd.cpp $(LOCAL_OBJS) $(UTIL_OBJS) $(DEV_OBJS)  | $(TGT)
	g++ $(CPPFLAGS) AdcHttpd.cpp \
              -I../           \
              -I$(PRU_INC)    \
              -L$(PRU_LIB)    \
                $(LOCAL_OBJS) \
                $(DEV_OBJS)   \
                $(UTIL_OBJS)  \
                $(EMUL_OBJS)  \
                $(LIBS) -lm -lfftw3 -lpthread -lmicrohttpd \
                -o $(TGT)/AdcHttpd

$(TGT)/Devs.o	: Devs.cpp Devs.h | $(TGT)
	g++ -I../ $(CPPFLAGS) -c Devs.cpp  -o $(TGT)/Devs.o

$(TGT)/Pse.o	: Pse.cpp Pse.h | $(TGT)
	g++ -I../ $(CPPFLAGS) -c Pse.cpp  -o $(TGT)/Pse.o

$(TGT)/Tim.o	: Tim.cpp Tim.h | $(TGT)
	g++ -I../ $(CPPFLAGS) -c Tim.cpp  -o $(TGT)/Tim.o

$(TGT)/His.o	: His.cpp His.h | $(TGT)
	g++ -I../ $(CPPFLAGS) -c His.cpp  -o $(TGT)/His.o

$(TGT)/Tok.o	: Tok.cpp Tok.h | $(TGT)
	g++ -I../ $(CPPFLAGS) -c Tok.cpp  -o $(TGT)/Tok.o

clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp


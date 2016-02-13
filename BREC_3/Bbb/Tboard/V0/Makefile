include ../common.mk

ifeq ($(TGT),arm)
    PRU_INC=/home/root/am335x_pru_package-master/pru_sw/app_loader/include
    PRU_LIB=/home/root/am335x_pru_package-master/pru_sw/app_loader/lib
    PASM=/home/root/am335x_pru_package-master/pru_sw/utils/pasm
else
    PRU_INC=../PruEmul/
    PRU_LIB=../PruEmul/x86
endif

UTIL_OBJS=../Util/$(TGT)/gpioutil.o \
          ../Util/$(TGT)/mcf.o \
          ../Iboard/$(TGT)/Iboard.o \
          ../Bdc/$(TGT)/Bdc.o \
          ../Fboard/$(TGT)/Fboard.o

LOCAL_OBJS=$(TGT)/ui2c.o \
           $(TGT)/mcp4725.o \
           $(TGT)/max2112.o

all	:
	make $(TGT)/Tutil


$(TGT)/Tutil	: Tutil.cpp $(LOCAL_OBJS) $(UTIL_OBJS) | $(TGT)
	g++ -I../ $(CPP_TGT_ARGS) $(CPPFLAGS) Tutil.cpp \
                $(LOCAL_OBJS) \
		$(UTIL_OBJS) \
		-L$(PRU_LIB) \
                -lprussdrv -lpthread -o $(TGT)/Tutil

$(TGT)/ui2c.o	: ui2c.cpp ui2c.h  | $(TGT)
	g++ -I../ -c $(CPP_TGT_ARGS) $(CPPFLAGS) ui2c.cpp \
                -o $(TGT)/ui2c.o

$(TGT)/mcp4725.o	: mcp4725.cpp mcp4725.h  | $(TGT)
	g++ -I../ -c $(CPP_TGT_ARGS) $(CPPFLAGS) mcp4725.cpp \
                -o $(TGT)/mcp4725.o

$(TGT)/max2112.o	: max2112.cpp max2112.h  | $(TGT)
	g++ -I../ -c $(CPP_TGT_ARGS) $(CPPFLAGS) max2112.cpp \
                -o $(TGT)/max2112.o

clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp

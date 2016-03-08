include ../MakeCommon.mk

ifeq ($(TGT),arm)
    XAPP058=/home/root/Xapp058
else
    XAPP058=/nfs-share/Xapp058
endif

UTIL_OBJS=
DEV_OBJS=
LOCAL_OBJS=

all	:
	make $(TGT)/xsvfLib.o
	make $(TGT)/xvcLib.o
	make $(TGT)/Sxvc
ifeq ($(TGT),arm)
endif

$(TGT)/xsvfLib.o	: xsvfLib.cpp | $(TGT)
	g++ -c -I../  -I$(XAPP058) xsvfLib.cpp -o $(TGT)/xsvfLib.o

$(TGT)/xvcLib.o		: xvcLib.cpp | $(TGT)
	g++ -c -I../ xvcLib.cpp -o $(TGT)/xvcLib.o

$(TGT)/Sxvc	: $(TGT)/jtag_bs_sim.o | $(TGT)
	g++ -I../ Sxvc.cpp $(TGT)/jtag_bs_sim.o \
            $(TGT)/xvcLib.o \
            -o $(TGT)/Sxvc

$(TGT)/jtag_bs_sim.o	: jtag_bs_sim.cpp | $(TGT)
	g++ -I../ -c -O3 jtag_bs_sim.cpp -o $(TGT)/jtag_bs_sim.o


clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp


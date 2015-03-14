
include ../common.mk

UTIL_OBJS=../Util/$(TGT)/mcf.o ../Util/$(TGT)/gpioutil.o
DEV_OBJS=../Bboard/$(TGT)/Bboard.o ../Adf4351/$(TGT)/Adf4351.o
LOCAL_OBJS=


all	:
	make $(TGT)/synHttpd

$(TGT)/synHttpd  : synHttpd.cpp $(LOCAL_OBJS) $(UTIL_OBJS) $(DEV_OBJS)  | $(TGT)
	g++ $(CPPFLAGS) synHttpd.cpp \
                $(LOCAL_OBJS) \
                $(DEV_OBJS) $(UTIL_OBJS) \
                -lm -lpthread -lmicrohttpd -o $(TGT)/synHttpd

clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp


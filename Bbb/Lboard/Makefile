include ../common.mk

LOCAL_OBJS=$(TGT)/Lboard.o 
UTIL_OBJS=../Util/$(TGT)/mcf.o ../Util/$(TGT)/cli.o ../Util/$(TGT)/net.o ../Util/$(TGT)/gpioutil.o ../Util/$(TGT)/SimpleTable.o
DEV_OBJS=../Iboard/$(TGT)/Iboard.o ../Mboard/$(TGT)/Mboard.o ../Adf4351/$(TGT)/Adf4351.o

all	:
	make $(TGT)/Lboard.o
	make $(TGT)/Lutil
	make $(TGT)/Lcal
	make $(TGT)/Lout

$(TGT)/Lboard.o : Lboard.cpp Lboard.h $(UTIL_OBJS) | $(TGT)
	g++ $(CPPFLAGS) -c Lboard.cpp -o $(TGT)/Lboard.o

$(TGT)/Lutil    : Lutil.cpp $(LOCAL_OBJS) $(UTIL_OBJS) $(DEV_OBJS)  | $(TGT)
	g++ $(CPPFLAGS) Lutil.cpp \
		$(LOCAL_OBJS) \
                $(DEV_OBJS) $(UTIL_OBJS) \
                -lm -lpthread -o $(TGT)/Lutil

$(TGT)/Lcal    : Lcal.cpp $(LOCAL_OBJS) $(UTIL_OBJS) $(DEV_OBJS)  | $(TGT)
	g++ $(CPPFLAGS) Lcal.cpp \
		$(LOCAL_OBJS) \
                $(DEV_OBJS) $(UTIL_OBJS) \
                -lm -lpthread -o $(TGT)/Lcal

$(TGT)/Lout    : Lout.cpp $(LOCAL_OBJS) $(UTIL_OBJS) $(DEV_OBJS)  | $(TGT)
	g++ $(CPPFLAGS) Lout.cpp \
		$(LOCAL_OBJS) \
                $(DEV_OBJS) $(UTIL_OBJS) \
                -lm -lpthread -o $(TGT)/Lout

clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp

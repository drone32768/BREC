include ../common.mk

LOCAL_OBJS=$(TGT)/Mboard.o 
UTIL_OBJS=../Util/$(TGT)/mcf.o ../Util/$(TGT)/cli.o ../Util/$(TGT)/net.o ../Util/$(TGT)/gpioutil.o
DEV_OBJS=../Iboard/$(TGT)/Iboard.o ../Adf4351/$(TGT)/Adf4351.o

all	:
	make $(TGT)/Mboard.o
	make $(TGT)/Mutil

$(TGT)/Mboard.o : Mboard.cpp Mboard.h $(UTIL_OBJS) | $(TGT)
	g++ $(CPPFLAGS) -c Mboard.cpp -o $(TGT)/Mboard.o

$(TGT)/Mutil    : Mutil.cpp $(LOCAL_OBJS) $(UTIL_OBJS) $(DEV_OBJS)  | $(TGT)
	g++ $(CPPFLAGS) Mutil.cpp \
		$(LOCAL_OBJS) \
                $(DEV_OBJS) $(UTIL_OBJS) \
                -lm -lpthread -o $(TGT)/Mutil

clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp

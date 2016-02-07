include ../common.mk

UTIL_OBJS=../Util/$(TGT)/gpioutil.o
DEV_OBJS=
LOCAL_OBJS=$(TGT)/Rboard.o 

all	:
	make $(TGT)/Rboard.o
	make $(TGT)/Rutil

$(TGT)/Rboard.o : Rboard.cpp Rboard.h $(UTIL_OBJS) | $(TGT)
	g++ $(CPPFLAGS) -c Rboard.cpp -o $(TGT)/Rboard.o

$(TGT)/Rutil    : Rutil.cpp $(LOCAL_OBJS) $(UTIL_OBJS) $(DEV_OBJS)  | $(TGT)
	g++ $(CPPFLAGS) Rutil.cpp \
                $(LOCAL_OBJS) \
                $(DEV_OBJS) $(UTIL_OBJS) \
                -lm -lpthread -o $(TGT)/Rutil

clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp

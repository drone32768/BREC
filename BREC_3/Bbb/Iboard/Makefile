include ../MakeCommon.mk

UTIL_OBJS=../Util/$(TGT)/gpioutil.o ../Util/$(TGT)/mcf.o
DEV_OBJS=
LOCAL_OBJS=$(TGT)/Iboard.o 

all	:
	make $(TGT)/Iboard.o
	make $(TGT)/Iutil

$(TGT)/Iboard.o : Iboard.cpp Iboard.h $(UTIL_OBJS) | $(TGT)
	g++ -I../ $(CPPFLAGS) -c Iboard.cpp -o $(TGT)/Iboard.o

$(TGT)/Iutil    : Iutil.cpp $(LOCAL_OBJS) $(UTIL_OBJS) $(DEV_OBJS)  | $(TGT)
	g++ -I../ $(CPPFLAGS) Iutil.cpp \
                $(LOCAL_OBJS) \
                $(DEV_OBJS) $(UTIL_OBJS) \
                -lm -lpthread -o $(TGT)/Iutil

clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp

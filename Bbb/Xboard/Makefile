include ../common.mk

UTIL_OBJS=../Util/$(TGT)/gpioutil.o ../Util/$(TGT)/mcf.o ../Iboard/$(TGT)/Iboard.o
DEV_OBJS=
LOCAL_OBJS=

all	:
	make $(TGT)/Xutil

$(TGT)/Xutil    : Xutil.cpp $(LOCAL_OBJS) $(UTIL_OBJS) $(DEV_OBJS)  | $(TGT)
	g++ $(CPPFLAGS) Xutil.cpp \
                $(LOCAL_OBJS) \
                $(DEV_OBJS) $(UTIL_OBJS) \
                -lm -lpthread -o $(TGT)/Xutil

clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp

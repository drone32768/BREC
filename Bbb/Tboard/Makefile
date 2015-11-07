include ../common.mk

UTIL_OBJS=../Util/$(TGT)/gpioutil.o \
          ../Util/$(TGT)/mcf.o \
          ../Iboard/$(TGT)/Iboard.o
DEV_OBJS=
#LOCAL_OBJS=$(TGT)/Tboard.o
LOCAL_OBJS=

all	:
	make $(TGT)/Tutil


$(TGT)/Tutil	: Tutil.cpp $(LOCAL_OBJS) $(UTIL_OBJS) $(DEV_OBJS)  | $(TGT)
	g++ $(CPP_TGT_ARGS) $(CPPFLAGS) Tutil.cpp \
                $(LOCAL_OBJS) \
		$(UTIL_OBJS) \
                -lpthread -o $(TGT)/Tutil

clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp

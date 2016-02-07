include ../common.mk

UTIL_OBJS=../Util/$(TGT)/gpioutil.o \
          ../Util/$(TGT)/mcf.o \
          ../Iboard/$(TGT)/Iboard.o
LOCAL_OBJS=$(TGT)/ui2c.o \
           $(TGT)/mcp4725.o \
           $(TGT)/max2112.o

all	:
	make $(TGT)/Tutil


$(TGT)/Tutil	: Tutil.cpp $(LOCAL_OBJS) $(UTIL_OBJS) | $(TGT)
	g++ $(CPP_TGT_ARGS) $(CPPFLAGS) Tutil.cpp \
                $(LOCAL_OBJS) \
		$(UTIL_OBJS) \
                -lpthread -o $(TGT)/Tutil

$(TGT)/ui2c.o	: ui2c.cpp ui2c.h  | $(TGT)
	g++ -c $(CPP_TGT_ARGS) $(CPPFLAGS) ui2c.cpp \
                -o $(TGT)/ui2c.o

$(TGT)/mcp4725.o	: mcp4725.cpp mcp4725.h  | $(TGT)
	g++ -c $(CPP_TGT_ARGS) $(CPPFLAGS) mcp4725.cpp \
                -o $(TGT)/mcp4725.o

$(TGT)/max2112.o	: max2112.cpp max2112.h  | $(TGT)
	g++ -c $(CPP_TGT_ARGS) $(CPPFLAGS) max2112.cpp \
                -o $(TGT)/max2112.o

clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp

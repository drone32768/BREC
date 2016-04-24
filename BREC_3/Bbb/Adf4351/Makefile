include ../MakeCommon.mk

UTIL_OBJS=../Util/$(TGT)/mcf.o \
          ../Util/$(TGT)/cli.o \
          ../Util/$(TGT)/net.o

all	:
	make $(TGT)/Adf4351.o

$(TGT)/Adf4351.o : Adf4351.cpp Adf4351.h $(UTIL_OBJS) | $(TGT)
	g++ -I../ $(CPPFLAGS) -c Adf4351.cpp -o $(TGT)/Adf4351.o

clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp

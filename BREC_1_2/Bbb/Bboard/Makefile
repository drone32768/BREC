include ../common.mk

UTIL_OBJS=../Util/$(TGT)/mcf.o ../Util/$(TGT)/cli.o ../Util/$(TGT)/net.o

all	:
	make $(TGT)/Bboard.o

$(TGT)/Bboard.o : Bboard.cpp Bboard.h $(UTIL_OBJS) | $(TGT)
	g++ $(CPPFLAGS) -c Bboard.cpp -o $(TGT)/Bboard.o

clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp

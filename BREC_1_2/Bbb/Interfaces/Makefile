
include ../common.mk

#------------------------------------------------------------------------------
all	:
#	echo host is $(HOSTARCH)
#	echo tgt  is $(TGT)
#	make $(TGT)/AdcIf.o

# Currently no .o's
$(TGT)/AdcIf.o	: AdcIf.h AdcIf.cpp | $(TGT)
	g++ $(CPPFLAGS) -c AdcIf.cpp -o $(TGT)/AdcIf.o

clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp

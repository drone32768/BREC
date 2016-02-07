
include ../common.mk

#------------------------------------------------------------------------------
all	:
#	echo host is $(HOSTARCH)
#	echo tgt  is $(TGT)
	make $(TGT)/mcf.o
	make $(TGT)/cli.o
	make $(TGT)/net.o
	make $(TGT)/gpioutil.o
	make $(TGT)/ccat
	make $(TGT)/SimpleTable.o

$(TGT)/ccat	:  ccat.cpp | $(TGT)
	g++ $(CPPFLAGS) ccat.cpp -o $(TGT)/ccat

$(TGT)/mcf.o	: mcf.h mcf.cpp | $(TGT)
	g++ $(CPPFLAGS) -c mcf.cpp -o $(TGT)/mcf.o

$(TGT)/cli.o	: cli.c cli.h | $(TGT)
	g++ $(CPPFLAGS) -c cli.c -o $(TGT)/cli.o

$(TGT)/net.o	: net.c net.h | $(TGT)
	g++ $(CPPFLAGS) -c net.c -o $(TGT)/net.o

$(TGT)/gpioutil.o	: gpioutil.cpp gpioutil.h | $(TGT)
	g++ $(CPPFLAGS) -c gpioutil.cpp -o $(TGT)/gpioutil.o

$(TGT)/SimpleTable.o	: SimpleTable.cpp SimpleTable.h | $(TGT)
	g++ $(CPPFLAGS) -c SimpleTable.cpp -o $(TGT)/SimpleTable.o

$(TGT)/SimpleTableTest	: SimpleTable.cpp SimpleTable.h | $(TGT)
	g++ $(CPPFLAGS) -DUNIT_TEST  SimpleTable.cpp -o $(TGT)/SimpleTableTest

clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp

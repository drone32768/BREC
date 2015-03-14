include ../common.mk

all	:
	make $(TGT)/SignalSim.o 
	make $(TGT)/pruss_emul.o 

$(TGT)/SignalSim.o	: SignalSim.cpp prussdrv.h  | $(TGT)
	g++ $(CPPFLAGS) -c SignalSim.cpp -o $(TGT)/SignalSim.o

$(TGT)/pruss_emul.o	: prussdrv.h pruss_emul.c   | $(TGT)
	gcc $(CPPFLAGS) -c pruss_emul.c -o $(TGT)/pruss_emul.o

clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp

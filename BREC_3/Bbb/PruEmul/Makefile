include ../common.mk

all	:
	make $(TGT)/pruss_emul.o 
	make $(TGT)/libprussdrv.a

$(TGT)/libprussdrv.a	: $(TGT)/pruss_emul.o | $(TGT)
	ar rc  $(TGT)/libprussdrv.a $(TGT)/pruss_emul.o 

$(TGT)/pruss_emul.o	: prussdrv.h pruss_emul.c   | $(TGT)
	gcc $(CPPFLAGS) -c pruss_emul.c -o $(TGT)/pruss_emul.o

clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp

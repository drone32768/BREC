include ../common.mk

ifeq ($(TGT),arm)
    PRU_INC=/home/root/am335x_pru_package-master/pru_sw/app_loader/include 
    PRU_LIB=/home/root/am335x_pru_package-master/pru_sw/app_loader/lib
    PASM=/home/root/am335x_pru_package-master/pru_sw/utils/pasm
else
    PRU_INC=../PruEmul/
endif

all	:
	echo target is $(TGT)
	make $(TGT)/Aboard.o
ifeq ($(TGT),arm)
	make $(TGT)/pructl
	make $(TGT)/pru00.bin
	make $(TGT)/pru01.bin
	cd Dt; make all
endif

$(TGT)/pructl   : pructl.c pructl.h pru00.hp | $(TGT)
	gcc -I$(PRU_INC) -L$(PRU_LIB) pructl.c \
		-o $(TGT)/pructl -lprussdrv -lpthread 

$(TGT)/pru00.bin : pru00.p pructl.h pru00.hp | $(TGT)
	${PASM} -V3 -b pru00.p
	mv pru00.bin $(TGT)/pru00.bin

$(TGT)/pru01.bin : pru01.p pructl.h pru00.hp | $(TGT)
	${PASM} -V3 -b pru01.p
	mv pru01.bin $(TGT)/pru01.bin

$(TGT)/Aboard.o : Aboard.cpp Aboard.h pru_images.h  | $(TGT)
	g++  $(CPPFLAGS) -c -I$(PRU_INC) Aboard.cpp -o $(TGT)/Aboard.o

ifeq ($(TGT),arm)
pru_images.h	: $(TGT)/pru00.bin $(TGT)/pru01.bin | $(TGT)
	../Util/$(TGT)/ccat -f $(TGT)/pru00.bin -n pru_image0 >  pru_images.h
	../Util/$(TGT)/ccat -f $(TGT)/pru01.bin -n pru_image1 >> pru_images.h
endif

clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp
